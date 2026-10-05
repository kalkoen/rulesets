//
// Created by 20193736 on 15/01/2026.
//

#include "MasterProblemBase.hpp"

#include <scip/cons_linear.h>
#include <scip/scipdefplugins.h>

#include "rsParams.h"
#include "typedef.h"
#include "Utils.hpp"
#include "binary/MasterProblemBinary.hpp"
#include "threshold/MasterProblemThreshold.hpp"


template <typename RSClause>
double MasterProblemBase<RSClause>::RuleComplexity(const RSRule& rule) const
{
    assert(rule_complexity_type == COMPLEXITY_ONE or rule_complexity_type == COMPLEXITY_CLAUSES or rule_complexity_type == COMPLEXITY_ONE_PLUS_CLAUSES);
    if (rule_complexity_type == COMPLEXITY_ONE)
    {
        return 1.0;
    }
    if (rule_complexity_type == COMPLEXITY_CLAUSES)
    {
        return rule.clauses.size();
    }
    if (rule_complexity_type == COMPLEXITY_ONE_PLUS_CLAUSES)
    {
        return 1.0 + rule.clauses.size();
    }
    return 0.0;
}

template <typename RSClause>
SCIP_RETCODE MasterProblemBase<RSClause>::CreateRSModel(SCIP* scip)
{
    SCIP_CALL(SCIPcreateProbBasic(scip, "MP"));

    SCIP_CALL(SCIPgetIntParam(scip, "rs/rule_complexity_type", &rule_complexity_type));
    SCIP_CALL(SCIPgetBoolParam(scip, "rs/cover_positive", &cover_positive));

    for (size_t i = 0; i < data.n_datapoints; i++)
    {
        if (IsPositive(i))
        {
            SCIP_VAR* var = nullptr;

            auto ub_zeta  = SCIPinfinity(scip);
            if (cover_positive)
            {
                ub_zeta = max_zeta - 1;
            }

            SCIPcreateVarBasic(scip, &var, ("zeta_" + std::to_string(i)).c_str(),
                               0.0, ub_zeta, 1.0, SCIP_VARTYPE_CONTINUOUS);

            SCIPaddVar(scip, var);
            zeta[i] = var;

            SCIP_CONS* cons = nullptr;
            auto coef = 1.0;

            SCIPcreateConsLinear(scip,
                                 &cons,
                                 ("fn_" + std::to_string(i)).c_str(),
                                 1,
                                 &var,
                                 &coef,
                                 max_zeta,
                                 SCIPinfinity(scip),
                                 true, true, true, true, true, false, true, true, false, false);

            fn_constraints[i] = cons;
            SCIPaddCons(scip, cons);
        }
    }


    SCIP_CALL(SCIPcreateConsLinear(scip,
        &complexity_constraint,
        "complexityConstr", // name
        0, // number of vars
        nullptr, // no vars yet
        nullptr, // no coefs yet
        -SCIPinfinity(scip), // lhs
        C == -1 ? SCIPinfinity(scip) : C,
        true, true, true, true, true, false, true, true, false, false));

    SCIP_CALL(SCIPaddCons(scip, complexity_constraint));

    return SCIP_OKAY;
}

template <typename RSClause>
SCIP_RETCODE MasterProblemBase<RSClause>::AddRule(
    SCIP* scip,
    const RSRule& rule,
    const bool asPricerVariable,
    const bool binary,
    uint32_t& ruleIdx)
{
    assert(rules.size() <= SIZE_MAX);

    if (rules.contains(rule))
    {
        std::cout << "!!! Cycling detected. Watch out." << std::endl;
    }

    double coeff_obj = 0.0;
    for (int i = 0; i < NumberOfDatapoints(); i++)
    {
        if (!IsPositive(i))
        {
            coeff_obj += Gamma(rule, i);
        }
    }

    SCIP_VAR* var = nullptr;
    if (binary)
    {
        SCIP_CALL(SCIPcreateVarBasic(
            scip,
            &var,
            ("w_" + std::to_string(rules.size())).c_str(),
            0,
            1,
            coeff_obj, SCIP_VARTYPE_BINARY));
    }
    else
    {
        SCIP_CALL(SCIPcreateVarBasic(
            scip,
            &var,
            ("w_" + std::to_string(rules.size())).c_str(),
            0,
            SCIPinfinity(scip),
            coeff_obj, SCIP_VARTYPE_CONTINUOUS));
    }

    if (asPricerVariable)
    {
        SCIP_CALL(SCIPaddPricedVar(scip, var, 0.0));
    }
    else
    {
        SCIP_CALL(SCIPaddVar(scip, var));
    }

    assert(w.size() == w_rules.size());
    w.push_back(var);
    w_rules.push_back(rule);
    ruleIdx = w.size() - 1;

    rules.insert(rule);

    for (size_t i = 0; i < NumberOfDatapoints(); i++)
    {
        double coeff_cons = Gamma(rule, i);

        if (IsPositive(i) and coeff_cons > 0)
        {
            SCIP_CONS* cons;
            if (asPricerVariable)
            {
                SCIP_CALL(SCIPgetTransformedCons(scip, fn_constraints[i], &cons));
            }
            else
              {
                cons = fn_constraints[i];
            }
            SCIP_CALL(SCIPaddCoefLinear(scip, cons, var, coeff_cons));
        }
    }

    SCIP_CONS* complCons;
    if (asPricerVariable)
    {
        SCIP_CALL(SCIPgetTransformedCons(scip, complexity_constraint, &complCons));
    }
    else
    {
        complCons = complexity_constraint;
    }
    SCIP_CALL(SCIPaddCoefLinear(scip, complCons, var, RuleComplexity(rule)));

    return SCIP_OKAY;
}


template <typename RSClause>
SCIP_RETCODE MasterProblemBase<RSClause>::AddRuleAndSubRules(
        SCIP* scip,
        const RSRule& rule,
        bool asPricerVariable,
        bool binary)
{
    std::vector<RSClause> clause_vector(rule.clauses.begin(), rule.clauses.end());

    std::vector<uint32_t> clause_indices(rule.size());
    std::iota(clause_indices.begin(), clause_indices.end(), 0);

    for (const auto& subset : Utils::subsets_up_to_size(clause_indices, rule.size(), {}))
    {
        RSRule sub_rule;
        for (auto clause_index: subset)
        {
            sub_rule.clauses.insert(clause_vector[clause_index]);
        }
        if (!rules.contains(sub_rule))
        {
            std::cout << "+ Rule: " << sub_rule << std::endl;
            uint32_t sub_rule_idx{};
            SCIP_CALL(AddRule(scip, sub_rule, asPricerVariable, binary, sub_rule_idx));
        }
    }

    return SCIP_OKAY;
}


template <typename RSClause>
SCIP_RETCODE MasterProblemBase<RSClause>::Free(SCIP* scip)
{
    if (scip != nullptr)
    {
        // Release variables
        for (auto& var : zeta | std::views::values)
        {
            SCIPreleaseVar(scip, &var);
        }
        for (auto& var : w)
        {
            SCIPreleaseVar(scip, &var);
        }

        // Release constraints
        for (auto& cons : fn_constraints | std::views::values)
        {
            SCIPreleaseCons(scip, &cons);
        }
        if (complexity_constraint != nullptr)
        {
            SCIPreleaseCons(scip, &complexity_constraint);
        }
    }
    return SCIP_OKAY;
}

template <typename RSClause>
SCIP_RETCODE MasterProblemBase<RSClause>::Solve(SCIP* scip)
{
    int method;
    SCIP_CALL(SCIPgetIntParam(scip, "rs/method", &method));

    if (method == METHOD_FULl)
    {
        std::cout << "Starting solve. Method: full IP." << std::endl;

        int D;
        SCIP_CALL(SCIPgetIntParam(scip, "rs/full/D", &D));

        for (auto candidate_rules = GetRulesUpToSize(D); const auto& rule : candidate_rules)
        {
            uint32_t rule_idx{};
            SCIP_CALL(AddRule(scip, rule, false, true, rule_idx));
        }

        std::cout << "Number of rules: " << rules.size() << std::endl;

//         SCIPprintOrigProblem(scip, NULL, NULL, false);

        SCIPsolve(scip);
        SCIPprintBestSol(scip, nullptr, false);

    }
    else if (method == METHOD_CG)
    {
        std::cout << "Starting solve. Method: Price then branch." << std::endl;

        SCIP* scip_cg;
        SCIP_CALL(SCIPcreate(&scip_cg));
        SCIP_CALL(SCIPincludeDefaultPlugins(scip_cg));
        SCIP_CALL(addRSParameters(scip_cg));
        SCIP_CALL(SCIPcopyParamSettings(scip, scip_cg));

        SCIP_Real timelimit;
        SCIP_Real cg_timelimit;
        SCIP_CALL(SCIPgetRealParam(scip, "limits/time", &timelimit));
        SCIP_CALL(SCIPgetRealParam(scip, "rs/cg_timelimit", &cg_timelimit));
        SCIPsetRealParam(scip_cg, "limits/time", std::min(timelimit, cg_timelimit));

        SCIPsetPresolving(scip_cg, SCIP_PARAMSETTING_OFF, true);
        SCIPsetSeparating(scip_cg, SCIP_PARAMSETTING_OFF, true);

        auto mp_cg = Copy();
        auto feature_names = data.feature_names;

        std::cout << "Starting pricing phase." << std::endl;
        mp_cg->CreateRSModel(
            scip_cg);


        unsigned int iterative_enabled;
        SCIP_CALL(SCIPgetBoolParam(scip_cg, "rs/iterative/enabled", &iterative_enabled));

        if (iterative_enabled)
        {
            SCIP_CALL(mp_cg->IncludeIterativePricer(scip_cg));
            std::cout << "Iterative pricer enabled." << std::endl;
        }

        unsigned int exact_enabled;
        SCIP_CALL(SCIPgetBoolParam(scip_cg, "rs/exact/enabled", &exact_enabled));

        if (exact_enabled)
        {
            SCIP_CALL(mp_cg->IncludeExactPricer(scip_cg));
            std::cout << "Exact pricer enabled." << std::endl;
        }

        unsigned int heuristic_enabled;
        SCIP_CALL(SCIPgetBoolParam(scip, "rs/heuristic/enabled", &heuristic_enabled));

        if (heuristic_enabled)
        {
            SCIP_CALL(mp_cg->IncludeHeuristicPricer(scip_cg));
            std::cout << "Heuristic pricer enabled." << std::endl;
        }


        SCIPsolve(scip_cg);
        SCIPprintBestSol(scip_cg, nullptr, false);

        cg_metrics.pricing_steps = std::move(mp_cg->cg_metrics.pricing_steps);
        cg_metrics.soltime_cg = SCIPgetSolvingTime(scip_cg);
        const auto cg_sol = SCIPgetBestSol(scip_cg);
        cg_metrics.relaxation_value = SCIPgetSolOrigObj(scip_cg, cg_sol);
        cg_metrics.status_cg = SCIPgetStatus(scip_cg);

        SCIPsetRealParam(scip, "limits/time", std::max(0.0, timelimit - cg_metrics.soltime_cg));

        std::cout << "Setting up branching phase." << std::endl;


        for (const auto& rule : mp_cg->rules)
        {
            if (IncludeSubRulesInBranchPhase())
            {
                SCIP_CALL(AddRuleAndSubRules(scip, rule, false, true));
            } else
            {
                std::cout << "+ Rule: " << rule << std::endl;
                uint32_t rule_idx{};
                SCIP_CALL(AddRule(scip, rule, false, true, rule_idx));
            }

        }

        mp_cg->Free(scip_cg);

        std::cout << "Starting branching phase." << std::endl;

        SCIPsolve(scip);
        SCIPprintBestSol(scip, nullptr, false);
    }

    return SCIP_OKAY;
}

template <typename RSClause>
nlohmann::json MasterProblemBase<RSClause>::ToJson(SCIP* scip)
{
    
    nlohmann::json j;

    SCIP_SOL* sol = SCIPgetBestSol(scip);
    j["status"] = SCIPgetStatus(scip);
    j["soltime"] = SCIPgetSolvingTime(scip);
    assert(sol != nullptr);
    j["obj"] = SCIPgetSolOrigObj(scip, sol);
    j["obj_relaxation"] = SCIPgetDualboundRoot(scip);
    j["dualbound"] = SCIPgetDualbound(scip);
    j["status_cg"] = cg_metrics.status_cg;
    j["soltime_cg"] = cg_metrics.soltime_cg;
    j["obj_cg"] = cg_metrics.relaxation_value;
    j["C"] = C;

    if (SCIPgetNSols(scip) > 0)
    {
        auto rules_j = nlohmann::json::array();
        auto relaxation = nlohmann::json::array();

        for (int i = 0; i < w.size(); ++i)
        {
            auto rule_json = RuleToJson(w_rules[i]);
            SCIP_Real solVal = SCIPgetSolVal(scip, sol, w[i]);
            if (SCIPisFeasGT(scip, solVal, 0.0))
            {
                rules_j.push_back(rule_json);
            }

            auto relaxation_json = rule_json;
            SCIP_Real relVal = SCIPvarGetLPSol(w[i]);
            if (SCIPisFeasGT(scip, relVal, 0.0))
            {
                relaxation_json.push_back(relVal);
                relaxation.push_back(relaxation_json);
            }
        }
        j["binary"] = rules_j;
        j["relaxation"] = relaxation;

        nlohmann::json j_steps = nlohmann::json::array();
        for (const auto& step : cg_metrics.pricing_steps)
        {
            j_steps.push_back(nlohmann::json{
                {"pricer", step.pricer},
                {"lp_objective", step.lp_objective},
                {"red_cost", step.lowest_red_cost},
                {"time", step.time}
            });
        }
        j["pricing_steps"] = j_steps;
    }
    return j;
}

template <typename RSClause>
SCIP_RETCODE MasterProblemBase<RSClause>::RecordPricingStep(SCIP* scip, int pricer, double lowest_red_cost)
{
    auto lp_objective = SCIPgetLPObjval(scip);
    auto solving_time = SCIPgetSolvingTime(scip);
    cg_metrics.pricing_steps.push_back({
        pricer,
        lp_objective,
        lowest_red_cost,
        solving_time
    });
    return SCIP_OKAY;
}

template <typename RSClause>
SCIP_RETCODE MasterProblemBase<RSClause>::ExtractShadowPrices(SCIP* scip, ShadowPrices& shadow_prices) const
{
    std::vector<SCIP_Real> mu;
    mu.resize(NumberOfDatapoints());
    for (const auto& [i, cons] : fn_constraints)
    {
        SCIP_CONS* transCons;
        SCIP_CALL(SCIPgetTransformedCons(scip, cons, &transCons));
        SCIP_ROW* row = SCIPconsGetRow(scip, transCons);
        mu[i] = SCIProwGetDualsol(row);
        assert(SCIPisFeasGE(scip, mu[i], 0.0) && "Mu values need to be non-negative");
    }
    SCIP_CONS* transComplCons;
    SCIP_CALL(SCIPgetTransformedCons(scip, complexity_constraint, &transComplCons));
    SCIP_ROW* row = SCIPconsGetRow(scip, transComplCons);
    SCIP_Real lambda = -SCIProwGetDualsol(row);
    assert(SCIPisFeasGE(scip, lambda, 0) && "Lambda values need to be non-negative");

    shadow_prices.mu = std::move(mu);
    shadow_prices.lambda = lambda;

    return SCIP_OKAY;
}

template <typename RSClause>
double MasterProblemBase<RSClause>::ComputeCGTimeLimit(SCIP* scip, const std::string& timelimit_setting)
{
    SCIP_Real exact_time_limit;
    SCIP_CALL(SCIPgetRealParam(scip, timelimit_setting.c_str(), &exact_time_limit));
    const double sol_time = SCIPgetSolvingTime(scip);
    SCIP_Real cg_timelimit;
    SCIP_CALL(SCIPgetRealParam(scip, "rs/cg_timelimit", &cg_timelimit));
    const double time_left = cg_timelimit - sol_time;
    const double time_limit = std::min(exact_time_limit, std::max(0.0, time_left));
    return time_limit;
}

template class MasterProblemBase<BooleanClause>;
template class MasterProblemBase<ThresholdClause>;