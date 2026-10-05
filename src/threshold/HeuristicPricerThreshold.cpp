//
// Created by 20193736 on 18/05/2026.
//

#include "MasterProblemThreshold.hpp"
#include "HeuristicPricerThreshold.hpp"

#include <chrono>

using MasterProblemT = MasterProblem<ThresholdClause>;


SCIP_RETCODE HeuristicPricer<ThresholdClause>::scip_init(SCIP* scip, SCIP_PRICER* pricer)
{
    MasterProblemT& mp = *mp_;
    SCIP_CALL(Init(scip, mp));

    n_datapoints = mp.NumberOfDatapoints();
    n_clauses = mp.NumberOfClauses();

    gamma_flat.resize(n_clauses * n_datapoints);

    nonzero_datapoints.resize(n_clauses);
    nontwo_datapoints.resize(n_clauses);

    const auto& clauses = mp.AllClauses();
    for (uint32_t l = 0; l < n_clauses; ++l)
    {
        auto& [feature, threshold] = clauses[l];

        for (uint32_t i = 0; i < n_datapoints; ++i)
        {
            auto gamma_il = mp.GammaClause(i, feature, threshold);
            gamma_flat[l * n_datapoints + i] = gamma_il;

            if (SCIPisFeasGT(scip, gamma_il, 0.0))
            {
                nonzero_datapoints[l].add(i);
            }
            if (SCIPisFeasLT(scip, gamma_il, 2.0))
            {
                nontwo_datapoints[l].add(i);
            }
        }
        nonzero_datapoints[l].runOptimize();
        nontwo_datapoints[l].runOptimize();
    }


    return SCIP_OKAY;
}

SCIP_RETCODE HeuristicPricer<ThresholdClause>::scip_redcost(SCIP* scip, SCIP_PRICER* pricer, SCIP_Real* lowerbound,
                                                            SCIP_Bool* stopearly, SCIP_RESULT* result)
{
    MasterProblemT& mp = *mp_;
    return Price(scip, mp, result);
}

double HeuristicPricer<ThresholdClause>::GammaClause(const uint32_t datapoint, const uint32_t clause) const
{
    return gamma_flat[clause * n_datapoints + datapoint];
}

Rule<ThresholdClause> HeuristicPricer<ThresholdClause>::ClauseIdxsToRule(const MasterProblemT& mp,
                                                                         const std::set<uint32_t>& clause_idxs) const
{
    const auto& all_clauses = mp.AllClauses();
    ThresholdRule rule;
    for (const uint32_t clause_idx : clause_idxs)
    {
        rule.clauses.insert(all_clauses[clause_idx]);
    }
    return rule;
}

const roaring::Roaring& HeuristicPricer<ThresholdClause>::PositiveDatapoints(
    const MasterProblemT& mp) const
{
    return mp.data.y;
}

const roaring::Roaring& HeuristicPricer<ThresholdClause>::NegativeDatapoints(
    const MasterProblemT& mp) const
{
    return mp.data.y_inv;
}

const roaring::Roaring& HeuristicPricer<ThresholdClause>::CoveredDatapoints(const MasterProblemT& mp,
                                                                            const uint32_t clause) const
{
    return nonzero_datapoints[clause];
}

const roaring::Roaring& HeuristicPricer<ThresholdClause>::NonFullyCoveredDatapoints(const MasterProblemT& mp,
    const uint32_t clause) const
{
    return nontwo_datapoints[clause];
}

bool HeuristicPricer<ThresholdClause>::KeepClause(
    SCIP* scip,
    const MasterProblemT& mp,
    const ShadowPrices& shadow_prices,
    const std::vector<std::vector<CandidateRule<ThresholdClause>>>& layers,
    const CandidateRuleT& base_candidate, const size_t depth,
    const uint32_t clause) const
{
    if (SCIPisFeasEQ(scip, shadow_prices.lambda, 0.0))
    {
        return true;
    }
    SCIP_Real val = shadow_prices.lambda * (static_cast<double>(base_candidate.clause_idxs.size()) + 1);
    for (const auto i : base_candidate.pos_covered_datapoints)
    {
        val -= shadow_prices.mu[i] * GammaClause(i, clause);
    }
    return SCIPisFeasLE(scip, val, 0.0);

    //
    // if (depth > 1)
    // {
    //     auto comp_redcost = base_candidate.reduced_cost;
    //     auto clauses = base_candidate.ExtractClauses(layers, depth-1);
    //     auto rule = ClausesToRule(mp, clauses);
    //     auto redcost = mp.ReducedCost(rule, shadow_prices);
    //
    //     std::cout << std::endl << " Check ";
    //     for (auto clause : clauses)
    //     {
    //         std::cout << clause << " ";
    //     }
    //     std::cout << " redcosts " << comp_redcost << " " << redcost << std::endl;;
    //     assert(SCIPisEQ(scip, comp_redcost, redcost));
    // }

    // return not std::ranges::contains(base_candidate.ExtractClauses(layers, depth), clause);
    return true;
}

void HeuristicPricer<ThresholdClause>::UpdateEmptyCandidate(const MasterProblemT& mp, const ShadowPrices& shadow_prices,
                                                            CandidateRuleT& empty_candidate) const
{
    empty_candidate.reduced_cost += empty_candidate.neg_covered_datapoints.cardinality() * 2.0;
    for (const auto i : empty_candidate.pos_covered_datapoints)
    {
        empty_candidate.reduced_cost -= shadow_prices.mu[i] * 2.0;
    }
}

void HeuristicPricer<ThresholdClause>::UpdateInferredCandidate(const MasterProblemT& mp,
                                                               const ShadowPrices& shadow_prices,
                                                               const std::vector<std::vector<CandidateRule<
                                                                   ThresholdClause>>>& layers,
                                                               CandidateRuleT& candidate, const size_t depth) const
{
    const auto& base_candidate = layers[depth - 1][candidate.parent_clause_idx];
    const auto& uncovered = NonFullyCoveredDatapoints(mp, candidate.new_clause_idx);

    if (mp.margin_width == 0)
    {
        for (const auto pos_datapoint :
             base_candidate.pos_covered_datapoints & uncovered)
        {
            candidate.reduced_cost += 2.0 * shadow_prices.mu[pos_datapoint];
        }

        for (const auto neg_datapoint :
             base_candidate.neg_covered_datapoints & uncovered)
        {
            candidate.reduced_cost -= 2.0;
        }
    }
    else
    {
        for (const auto pos_datapoint :
             base_candidate.pos_covered_datapoints & uncovered)
        {
            const auto latest_gamma = GetLatestGamma(mp, layers, candidate, depth, shadow_prices, pos_datapoint);
            const auto possible_new_gamma = GammaClause(pos_datapoint, candidate.new_clause_idx);

            if (const auto diff = possible_new_gamma - latest_gamma; diff < 0)
            {
                // std::cout << "Pos: " << latest_gamma << " " << possible_new_gamma << " feature " << candidate.new_clause << " " << pos_datapoint << std::endl;
                // std:: cout << "Parent: " << base_candidate.new_clause << "clause " << candidate.new_clause << " datapoint " << pos_datapoint << " diff " << diff << std::endl;
                candidate.reduced_cost -= diff * shadow_prices.mu[pos_datapoint];
                candidate.custom_data.latest_gammas[pos_datapoint] = possible_new_gamma;
            }
        }

        for (const auto neg_datapoint :
             base_candidate.neg_covered_datapoints & uncovered)
        {
            const auto latest_gamma = GetLatestGamma(mp, layers, candidate, depth, shadow_prices, neg_datapoint);
            const auto possible_new_gamma = GammaClause(neg_datapoint, candidate.new_clause_idx);

            // std::cout << "Neg: " << latest_gamma << " " << possible_new_gamma << " feature " << candidate.new_clause << " " << neg_datapoint << std::endl;

            if (const auto diff = possible_new_gamma - latest_gamma; diff < 0)
            {
                // std:: cout << "Parent: " << base_candidate.new_clause << "clause " << candidate.new_clause << " datapoint " << neg_datapoint << " diff " << diff << std::endl;

                candidate.reduced_cost += diff;
                candidate.custom_data.latest_gammas[neg_datapoint] = possible_new_gamma;
            }
        }
    }

    const auto added_feature = mp.AllClauses()[candidate.new_clause_idx].feature;
    const auto& to_deactivate = mp.ClauseIndicesByFeature(added_feature);
    candidate.inactive_clauses.addMany(to_deactivate.size(), to_deactivate.data());

    for (const auto forbidden_feature : mp.data.forbidden_feature_combinations[added_feature])
    {
        const auto& to_also_deactivate = mp.ClauseIndicesByFeature(forbidden_feature);
        // std::cout << "For clause " << candidate.new_clause << " feature " << added_feature << " << added_feature << " << "deactivating feature " << forbidden_feature << " with clauses ";
        // for (auto l : to_also_deactivate)
        // {
        //     std::cout << l << " ";
        // }
        // std::cout << "\n";
        candidate.inactive_clauses.addMany(to_also_deactivate.size(), to_also_deactivate.data());
    }

    // std::cout << "Clause " << candidate.new_clause << " = feature " << added_feature << " was just added, deactivating ";
    // for (auto l : to_deactivate)
    // {
    //     std::cout << l << " ";
    // }
    // std::cout << std::endl;

    candidate.inactive_clauses.runOptimize();
}

double HeuristicPricer<ThresholdClause>::GetLatestGamma(const MasterProblemT& mp,
                                                        const std::vector<std::vector<CandidateRule<ThresholdClause>>>&
                                                        layers,
                                                        const CandidateRuleT& candidate,
                                                        uint32_t depth,
                                                        const ShadowPrices& shadow_prices,
                                                        const uint32_t datapoint)
{
    auto parent_idx = candidate.parent_clause_idx;
    while (depth > 1)
    {
        const auto& parent = layers[depth - 1][parent_idx];
        const auto& latest_gammas = parent.custom_data.latest_gammas;
        if (const auto kv = latest_gammas.find(datapoint); kv != latest_gammas.end())
            return kv->second;

        parent_idx = parent.parent_clause_idx;
        --depth;
    }
    return 2.0;
}
