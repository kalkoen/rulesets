//
// Created by 20193736 on 10/04/2026.
//

#include "MasterProblemThreshold.hpp"

#include <unordered_set>
#include <scip/cons_linear.h>

#include "../IterativePricer.hpp"
#include "ExactPricerThreshold.hpp"
#include "HeuristicPricerThreshold.hpp"

#include "../Utils.hpp"

#include "rapidcsv.h"
#include "../typedef.h"

SCIP_RETCODE MasterProblem<ThresholdClause>::CreateRSModel(
    SCIP *scip) {
    SCIP_CALL(MasterProblemBase<ThresholdClause>::CreateRSModel(scip));

    char *c_q;
    SCIP_CALL(SCIPgetStringParam(scip, "rs/threshold/quantiles", &c_q));
    const std::string str_q(c_q);
    quantile_levels = Utils::parse_q(str_q);

    std::cout << "Quantile levels: " << quantile_levels << std::endl;
    const size_t n_clauses_estimate = NumberOfFeatures() * quantile_levels.size();
    all_clauses.reserve(n_clauses_estimate);
    clause_idx_by_feature.resize(NumberOfFeatures());

    for (uint32_t j = 0; j < NumberOfFeatures(); ++j) {
        clause_idx_by_feature[j].reserve(quantile_levels.size());
        for (const auto quantiles = Utils::get_quantiles(Xj(j), quantile_levels); const auto q: quantiles) {
            clause_idx_by_feature[j].push_back(all_clauses.size());
            ThresholdClause c = {
                .feature = j,
                .threshold = q
            };
            std::cout << "Clause " << all_clauses.size() << ": " << c << std::endl;
            all_clauses.push_back(c);
        }
    }

    SCIP_CALL(SCIPgetBoolParam(scip, "rs/threshold/nonadditive", &nonadditive));

    return SCIP_OKAY;
}

SCIP_RETCODE MasterProblem<ThresholdClause>::AddRule(
    SCIP *scip,
    const ThresholdRule &rule,
    const bool asPricerVariable,
    const bool binary,
    uint32_t &ruleIdx) {
    SCIP_CALL(MasterProblemBase<ThresholdClause>::AddRule(scip, rule, asPricerVariable, binary, ruleIdx));

    if (nonadditive and margin_width > 0 and not asPricerVariable) {
        const auto w_var = w[ruleIdx];

        data_nonadditive.resize(ruleIdx+1);

        // Adding constraints for newly added rule, so where k' = ruleIdx
        for (uint32_t i = 0; i < NumberOfDatapoints(); ++i) {
            if (IsPositive(i)) {
                const auto zeta_var = zeta[i];

                DataNonadditive data_ik_prime{};
                data_ik_prime.eta = -Gamma(rule, i);

                std::vector<SCIP_VAR *> vars = {zeta_var};
                std::vector<double> coeffs = {1.0};
                vars.reserve(NumberOfClauses());
                coeffs.reserve(NumberOfClauses());

                for (int k = 0; k < ruleIdx; ++k) {
                    const auto &data_ik = data_nonadditive[k][i];
                    if (SCIPisLT(scip, data_ik.eta, data_ik_prime.eta)) {
                        // std::cout << "Adding var" << data_ik_prime.eta - data_ik.eta << " to k " << k << " k' " << ruleIdx << std::endl << "i " << i;
                        vars.push_back(w[k]);
                        coeffs.push_back((data_ik_prime.eta) - (data_ik.eta));
                    }
                }

                SCIPcreateConsLinear(scip,
                                     &data_ik_prime.cons,
                                     ("fnt_" + std::to_string(ruleIdx) + "_" + std::to_string(i)).c_str(),
                                     vars.size(),
                                     vars.data(),
                                     coeffs.data(),
                                     2 + data_ik_prime.eta,
                                     SCIPinfinity(scip),
                                     true,
                                     true,
                                     true,
                                     true,
                                     true,
                                     false,
                                     true,
                                     true,
                                     false,
                                     false);

                SCIPaddCons(scip, data_ik_prime.cons);

                data_nonadditive[ruleIdx][i] = data_ik_prime;
            }
        }

        // Adding newly added rule to existing constraints (so k = ruleIdx)
        for (int k_prime = 0; k_prime < ruleIdx; ++k_prime) {
            const auto &data_k_prime = data_nonadditive[k_prime];
            for (uint32_t i = 0; i < NumberOfDatapoints(); ++i) {
                if (IsPositive(i)) {
                    const auto &data_ik_prime = data_k_prime.at(i);
                    const auto &data_ik = data_nonadditive[ruleIdx][i];

                    if (SCIPisLT(scip, data_ik.eta, data_ik_prime.eta)) {
                        // std::cout << "Adding coeff" << data_ik_prime.eta - data_ik.eta << " to k' " << k_prime << " k " << ruleIdx << std::endl;
                        SCIPaddCoefLinear(scip, data_ik_prime.cons, w_var, data_ik_prime.eta - data_ik.eta);
                    }
                }
            }
        }
    }

    return SCIP_OKAY;
}

SCIP_RETCODE MasterProblem<ThresholdClause>::Free(SCIP *scip) {
    for (auto &data_k: data_nonadditive) {
        for (auto &[gamma, cons]: data_k | std::views::values) {
            SCIP_CALL(SCIPreleaseCons(scip, &cons));
        }
    }

    return MasterProblemBase<ThresholdClause>::Free(scip);
}

double MasterProblem<ThresholdClause>::GammaClause(
    const size_t datapoint,
    const size_t feature,
    const double threshold) const {
    if (margin_width == 0)
    {
        return Xj(feature)[datapoint] >= threshold ? 2.0 : 0.0;
    }
    return std::clamp(1 + 1 / margin_width * (Xj(feature)[datapoint] - threshold), 0.0, 2.0);
}

double MasterProblem<ThresholdClause>::Gamma(const ThresholdRule &rule, const size_t datapoint) const {
    double min_val = std::numeric_limits<double>::max();
    for (const auto &clause: rule.clauses) {
        if (const double val = GammaClause(datapoint, clause.feature, clause.threshold); val < min_val)
            min_val = val;
    }
    return min_val;
}

bool MasterProblem<ThresholdClause>::RuleSatisfiesDataPoint(const ThresholdRule &rule, const size_t datapoint) const {
    return std::ranges::all_of(rule.clauses, [&](const ThresholdClause &clause) {
        return SatisfiesThreshold(datapoint, clause.feature, clause.threshold);
    });
}


double MasterProblem<ThresholdClause>::ReducedCost(const ThresholdRule &rule, const ShadowPrices &shadow_prices) const {
    double reduced_cost = shadow_prices.lambda * RuleComplexity(rule);
    for (int i = 0; i < NumberOfDatapoints(); i++) {
        if (IsPositive(i)) {
            reduced_cost -= shadow_prices.mu[i] * Gamma(rule, i);
        } else {
            reduced_cost += Gamma(rule, i);
        }
    }
    return reduced_cost;
}

bool MasterProblem<ThresholdClause>::IsPositive(const size_t datapoint) const {
    return data.y.contains(datapoint);
}

bool MasterProblem<ThresholdClause>::SatisfiesThreshold(const size_t datapoint, const size_t feature,
                                                        const double threshold) const {
    return GetVector(data.X, feature)[datapoint] >= threshold;
}

nlohmann::json MasterProblem<ThresholdClause>::RuleToJson(const ThresholdRule &rule) const {
    auto j = nlohmann::json::array();

    for (const auto &[feature, threshold]: rule.clauses) {
        const auto &name = data.feature_names[feature];
        std::string label = name + ">=" + std::to_string(threshold) + "";
        j.push_back(label);
    }

    return j;
}

Generator<Rule<ThresholdClause> > MasterProblem<ThresholdClause>::GetRulesUpToSize(const int k) {
    std::vector<uint32_t> feature_idxs(NumberOfFeatures());
    std::iota(feature_idxs.begin(), feature_idxs.end(), 0);
    auto subsets = Utils::subsets_up_to_size(feature_idxs, k, data.forbidden_feature_combinations);

    for (const auto &subset: subsets) {
        for (const auto &clause_set: Utils::cartesian_product(clause_idx_by_feature, subset)) {
            std::set<ThresholdClause> clauses;
            std::ranges::transform(clause_set,
                                   std::inserter(clauses, clauses.end()),
                                   [&](uint32_t i) {
                                       return all_clauses[i];
                                   });

            // std::cout << ThresholdRule(clauses) << std::endl;

            co_yield ThresholdRule(std::move(clauses));
        }
    }
}


MasterProblemData<ThresholdClause> MasterProblemData<ThresholdClause>::load_csv(
    const std::string &filenameX,
    const std::string &filenameY,
    int negation_mode) {
    MasterProblemData<ThresholdClause> mp_data;

    const rapidcsv::Document X_doc(filenameX);
    std::vector<std::string> column_names = X_doc.GetColumnNames();

    const rapidcsv::Document y_doc(filenameY);
    assert(y_doc.GetColumnCount() == 1 && "y file must have a single column");

    auto y_int = y_doc.GetColumn<int>(0);
    std::vector<uint32_t> indices;
    indices.reserve(y_int.size() / 2);
    assert(y_int.size() < std::numeric_limits<uint32_t>::max());
    for (size_t i = 0; i < y_int.size(); ++i) {
        if (y_int[i] > 0) {
            indices.push_back(static_cast<uint32_t>(i));
        }
    }
    mp_data.n_datapoints = y_int.size();
    mp_data.y.addMany(indices.size(), indices.data());
    mp_data.y.runOptimize();

    mp_data.y_inv = mp_data.y;
    mp_data.y_inv.flip(0, y_int.size());

    std::vector<std::vector<double> > X;

    std::vector<std::vector<double> > gamma_feature;

    for (const std::string &header: column_names) {
        std::vector<double> x = X_doc.GetColumn<double>(header);
        assert(x.size() == y_int.size());

        X.push_back(x);

        mp_data.feature_names.push_back(header);
    }

    mp_data.n_features = X.size();
    mp_data.feature_names = column_names;

    if (negation_mode != NEGATE_OFF) {
        if (negation_mode == NEGATE_NO_MIXING) {
            mp_data.forbidden_feature_combinations.resize(mp_data.n_features * 2);
        }

        X.resize(mp_data.n_features * 2);
        mp_data.feature_names.resize(mp_data.n_features * 2);

        for (int j = 0; j < mp_data.n_features; ++j) {
            if (negation_mode == NEGATE_NO_MIXING) {
                mp_data.forbidden_feature_combinations[j].insert(mp_data.n_features + j);
                mp_data.forbidden_feature_combinations[mp_data.n_features + j].insert(j);
            }

            std::vector<double> neg(X[j].size());
            std::ranges::transform(X[j], neg.begin(), std::negate<>());
            X[mp_data.n_features + j] = neg;
            mp_data.feature_names[mp_data.n_features + j] = "-" + mp_data.feature_names[j];
        }
        mp_data.n_features = X.size();
    }


    auto X_view = X | std::views::join;
    std::vector X_flat(X_view.begin(), X_view.end());
    mp_data.X = std::move(X_flat);

    return mp_data;
}

SCIP_RETCODE MasterProblem<ThresholdClause>::IncludeIterativePricer(SCIP *scip) {
    auto *iterative_pricer = new IterativePricer<ThresholdClause>(this, scip);
    SCIP_CALL(SCIPincludeObjPricer(scip, iterative_pricer, true));
    SCIP_PRICER *pricerPtr = SCIPfindPricer(scip, "IterativePricer");
    assert(pricerPtr != nullptr);
    SCIP_CALL(SCIPactivatePricer(scip, pricerPtr));
    return SCIP_OKAY;
}


SCIP_RETCODE MasterProblem<ThresholdClause>::IncludeExactPricer(SCIP *scip) {
    auto *exact_pricer = new ExactPricerThreshold(this, scip);
    SCIP_CALL(SCIPincludeObjPricer(scip, exact_pricer, true));
    SCIP_PRICER *pricerPtr = SCIPfindPricer(scip, "ExactPricer_threshold");
    assert(pricerPtr != nullptr);
    SCIP_CALL(SCIPactivatePricer(scip, pricerPtr));
    return SCIP_OKAY;
}

SCIP_RETCODE MasterProblem<ThresholdClause>::IncludeHeuristicPricer(SCIP *scip) {
    auto *heuristic_pricer = new HeuristicPricer<ThresholdClause>(this, scip);
    SCIP_CALL(SCIPincludeObjPricer(scip, heuristic_pricer, true));
    SCIP_PRICER *pricerPtr = SCIPfindPricer(scip, "HeuristicPricer");
    assert(pricerPtr != nullptr);
    SCIP_CALL(SCIPactivatePricer(scip, pricerPtr));
    return SCIP_OKAY;
}

bool MasterProblem<ThresholdClause>::IncludeSubRulesInBranchPhase()
{
    return nonadditive and margin_width > 0;
}

nlohmann::json MasterProblem<ThresholdClause>::ToJson(SCIP *scip) {
    auto json = MasterProblemBase<ThresholdClause>::ToJson(scip);
    json["margin_width"] = margin_width;
    json["quantile_levels"] = quantile_levels;
    return json;
}
