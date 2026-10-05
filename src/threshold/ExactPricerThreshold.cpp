#include "ExactPricerThreshold.hpp"
#include "MasterProblemThreshold.hpp"

#include <algorithm>
#include "scip/scipdefplugins.h"
#include <iostream>
#include <cstdio>
#include "../typedef.h"
#include "../Utils.hpp"
#include "separation/MinConshdlr.hpp"

using MasterProblemT = MasterProblem<ThresholdClause>;


SCIP_RETCODE ExactPricerThreshold::scip_init(SCIP* scip, SCIP_PRICER* pricer)
{
    const MasterProblemT& mp = *mp_;

    SCIP_CALL(SCIPcreate(&subscip));
    SCIP_CALL(SCIPincludeDefaultPlugins(subscip));

    SCIPsetMessagehdlrQuiet(subscip, false);

    SCIP_CALL(SCIPcreateProbBasic(subscip, "PP"));

    // SCIP_CALL(SCIPsetIntParam(subscip, "presolving/maxrounds", 0));
    // SCIP_CALL(SCIPsetIntParam(subscip, "misc/usesymmetry", 0));
    // // Prioritize heuristics
    // SCIP_CALL(SCIPsetHeuristics(scip, SCIP_PARAMSETTING_AGGRESSIVE, true));
    //
    // // Deprioritize cuts/separation since you don't need to prove optimality
    // SCIP_CALL(SCIPsetSeparating(scip, SCIP_PARAMSETTING_FAST, true));
    //
    // // Don't waste time on presolving
    // SCIP_CALL(SCIPsetPresolving(scip, SCIP_PARAMSETTING_FAST, true));

    SCIPsetEmphasis(scip, SCIP_PARAMEMPHASIS_OPTIMALITY, true);


    SCIP_CALL(SCIPgetIntParam(scip, "rs/exact/D", &D));

    g.resize(mp.NumberOfDatapoints());
    for (int i = 0; i < mp.NumberOfDatapoints(); ++i)
    {
        const auto obj = mp.IsPositive(i) ? 0.0 : 1.0;
        SCIP_VAR* var = nullptr;
        SCIP_CALL(SCIPcreateVarBasic(subscip, &var, ("g_" + std::to_string(i)).c_str(),
            0.0, 2.0, obj, SCIP_VARTYPE_CONTINUOUS));
        SCIP_CALL(SCIPaddVar(subscip, var));
        g[i] = var;
    }

    const auto& clauses = mp.AllClauses();
    z.resize(clauses.size());
    for (auto l = 0; l < clauses.size(); ++l)
    {
        const auto& [feature, threshold] = clauses[l];
        SCIP_VAR* var = nullptr;
        SCIP_CALL(SCIPcreateVarBasic(subscip, &var,
            ("z_" + std::to_string(l)).c_str(),
            0, 1, 0.0, SCIP_VARTYPE_BINARY));
        SCIP_CALL(SCIPaddVar(subscip, var));
        z[l] = var;

        std::cout << "Clause " << l << " : " << clauses[l] << std::endl;
    }

    SCIP_CONS* cons_z_total = nullptr;
    std::vector<SCIP_Real> coeffs_z_total(z.size(), 1.0);
    SCIP_CALL(SCIPcreateConsBasicLinear(subscip, &cons_z_total, "con_z_total",
        static_cast<int>(z.size()), z.data(), coeffs_z_total.data(), -SCIPinfinity(subscip), D));
    SCIP_CALL(SCIPaddCons(subscip, cons_z_total));
    SCIP_CALL(SCIPreleaseCons(subscip, &cons_z_total));

    for (size_t j = 0; j < mp.NumberOfFeatures(); ++j)
    {
        std::vector<SCIP_VAR*> zj;
        for (const auto l : mp.ClauseIndicesByFeature(j))
            zj.push_back(z[l]);

        for (const auto forbidden_feature : mp.data.forbidden_feature_combinations[j])
        {
            for (const auto forbidden_l : mp.ClauseIndicesByFeature(forbidden_feature))
            {
                zj.push_back(z[forbidden_l]);
            }
        }

        if (zj.empty()) continue;

        std::vector<double> coeffs(zj.size(), 1);
        SCIP_CONS* cons = nullptr;
        SCIP_CALL(SCIPcreateConsBasicLinear(subscip, &cons,
            ("con_atmostone_" + std::to_string(j)).c_str(),
            static_cast<int>(zj.size()), zj.data(), coeffs.data(), 0.0, 1.0));
        SCIP_CALL(SCIPaddCons(subscip, cons));
        SCIP_CALL(SCIPreleaseCons(subscip, &cons));
    }

    point_data.resize(mp.NumberOfDatapoints());
    for (int i = 0; i < mp.NumberOfDatapoints(); ++i)
    {
        PointData gd;
        gd.type = mp.IsPositive(i) ? MinType::UPPER_BOUND : MinType::LOWER_BOUND;
        gd.gammas_unsorted.resize(mp.NumberOfClauses());
        gd.gammas_sorted.resize(mp.NumberOfClauses());
        gd.sorted_index_map.resize(mp.NumberOfClauses());
        auto& all_clauses = mp.AllClauses();
        for (int l = 0; l < all_clauses.size(); ++l)
        {
            const auto& [feature, threshold] = all_clauses[l];
            const auto gamma_il = mp.GammaClause(i, feature, threshold);
            gd.gammas_unsorted[l] = gamma_il;
            gd.gammas_sorted[l] = gamma_il;
            gd.sorted_index_map[l] = l;
        }

        Utils::sort_together(gd.gammas_sorted, gd.sorted_index_map);

        point_data[i] = std::move(gd);
    }


    auto min_conshdlr = new MinConshdlr(this, subscip);
    SCIP_CALL(SCIPincludeObjConshdlr(subscip, min_conshdlr, true));

    for (uint32_t i = 0; i < mp.NumberOfDatapoints(); ++i)
    {
        SCIP_CONS* cons_i = nullptr;
        SCIP_CALL(MinConshdlr::createCons(
            this,
            subscip,
            &cons_i,
            ("mincons_" + std::to_string(i)).c_str(),
            i));
        SCIP_CALL(SCIPaddCons(subscip, cons_i));
        SCIP_CALL(SCIPreleaseCons(subscip, &cons_i));
    }

    SCIP_CALL(SCIPsetIntParam(subscip, "heuristics/alns/freq", -1));

    std::cout << "Exact Pricer Initialized." << std::endl;
    return SCIP_OKAY;
}


SCIP_RETCODE ExactPricerThreshold::scip_redcost(
    SCIP* scip, SCIP_PRICER* pricer,
    SCIP_Real* lowerbound, SCIP_Bool* stopearly, SCIP_RESULT* result)
{
    MasterProblemT& mp = *mp_;
    std::cout << "Exact Pricer Called." << std::endl;

    ShadowPrices shadow_prices;
    SCIP_CALL(mp.ExtractShadowPrices(scip, shadow_prices));
    const auto& lambda = shadow_prices.lambda;

    SCIP_CALL(UpdatePricingProblem(mp, shadow_prices));

    // SCIP_CALL(SCIPwriteOrigProblem(subscip, "pricing_problem.lp", "lp", FALSE));

    // SCIPwriteOrigProblem(subscip, "model.lp", nullptr, FALSE);

    // std::cout << "PRINTING ORIGINAL PROBLEM" << std::endl;

    SCIP_Real exact_time_limit;
    SCIP_CALL(SCIPgetRealParam(scip, "rs/exact/timelimit", &exact_time_limit));
    const double sol_time = SCIPgetSolvingTime(scip);
    SCIP_Real cg_timelimit;
    SCIP_CALL(SCIPgetRealParam(scip, "rs/cg_timelimit", &cg_timelimit));
    SCIPsetRealParam(subscip, "limits/time",
                     std::min(exact_time_limit, std::max(0.0, cg_timelimit - sol_time)));

    SCIPinfoMessage(scip, nullptr, "SCIP stage before solve: %d\n", SCIPgetStage(subscip));

    // SCIP_CALL(SCIPsetObjlimit(subscip, -lambda));
    // SCIP_CALL(SCIPsetRealParam(subscip, "limits/dual", -lambda));
    // SCIP_CALL(SCIPsetIntParam(subscip, "limits/solutions", 1));

    std::cout << "Lambda: " << lambda << std::endl;

    SCIP_CALL(SCIPsolve(subscip));

    // // After SCIPsolve
    // SCIP_SOL* lpsol = SCIPgetBestSol(subscip); // or construct from LP values
    // SCIP_CALL(SCIPprintSol(subscip, lpsol, NULL, FALSE));

    if (SCIPgetStatus(subscip) == SCIP_STATUS_INFEASIBLE)
    {
        SCIPinfoMessage(subscip, nullptr, "=== INFEASIBLE ===\n");
        SCIP_CALL(SCIPprintOrigProblem(subscip, nullptr, "cip", FALSE));
        SCIP_CALL(SCIPprintTransProblem(subscip, nullptr, "cip", FALSE));
        SCIP_CALL(SCIPprintStatistics(subscip, nullptr));
    }

    *result = ::SCIP_DIDNOTRUN;

    if (SCIP_SOL* sol = SCIPgetBestSol(subscip); sol != nullptr)
    {
        double soltime = SCIPgetSolvingTime(subscip);
        const SCIP_Real obj = SCIPgetSolOrigObj(subscip, sol);

        double real_obj = obj;
        if (mp_->RuleComplexityType() == COMPLEXITY_ONE or mp_->RuleComplexityType() == COMPLEXITY_ONE_PLUS_CLAUSES)
        {
            real_obj += lambda;
        }

        SCIPprintSol(subscip, sol, nullptr, true);
        if (SCIPisDualfeasNegative(scip, real_obj))
        {
            ThresholdRule rule;
            for (size_t l = 0; l < z.size(); ++l)
            {
                const auto& z_var = z[l];
                const auto& clause = mp.AllClauses()[l];

                if (const SCIP_Real val = SCIPgetSolVal(subscip, sol, z_var); SCIPisEQ(subscip, val, 1))
                    rule.clauses.insert(clause);
            }
            std::cout << "Rule found: " << rule << std::endl;
            if (!rule.empty())
            {
                const auto red_cost = mp.ReducedCost(rule, shadow_prices);

                std::cout << "+ Rule: " << rule << "\t Reduced cost: " << red_cost << " / " << real_obj <<
                    std::endl;

                uint32_t rule_idx{};
                SCIP_CALL(mp.AddRule(scip, rule, true, false, rule_idx));
                
                *result = SCIP_SUCCESS;
                mp.RecordPricingStep(scip, PRICER_EXACT, red_cost);
            }
            else
            {
                std::cout << "Empty rule!" << std::endl;
                *result = SCIP_SUCCESS;
            }
            std::cout << ">Solve time: " + std::to_string(soltime) + "\n";
        }
        else
        {
            std::cout << "Rule found, but no reduced cost \n";
            *result = SCIP_SUCCESS;
        }
    }
    else
    {
        std::cout << "No solution found.";
    }

    return SCIP_OKAY;
}

SCIP_RETCODE ExactPricerThreshold::scip_farkas(
    SCIP* scip, SCIP_PRICER* pricer, SCIP_RESULT* result)
{
    *result = ::SCIP_DIDNOTRUN;
    return SCIP_OKAY;
}

SCIP_RETCODE ExactPricerThreshold::scip_exit(SCIP* scip, SCIP_PRICER* pricer)
{
    SCIP_CALL(SCIPfreeTransform(subscip));
    for (auto var : g)
        SCIP_CALL(SCIPreleaseVar(subscip, &var));
    for (auto var : z)
        SCIP_CALL(SCIPreleaseVar(subscip, &var));
    SCIPfree(&subscip);
    return SCIP_OKAY;
}

SCIP_RETCODE ExactPricerThreshold::UpdatePricingProblem(
    const MasterProblemT& mp, const ShadowPrices& shadow_prices) const
{
    SCIP_CALL(SCIPfreeTransform(subscip));
    for (const auto i : mp.data.y)
        SCIP_CALL(SCIPchgVarObj(subscip, g[i], -shadow_prices.mu[i]));
    if (mp_->RuleComplexityType() == COMPLEXITY_CLAUSES or mp_->RuleComplexityType() == COMPLEXITY_ONE_PLUS_CLAUSES)
    {
        for (const auto zj : z)
            SCIP_CALL(SCIPchgVarObj(subscip, zj, shadow_prices.lambda));
    }
    return SCIP_OKAY;
}
