//
// Created by 20193736 on 28/06/2026.
//

#include "IterativePricer.hpp"

#include "binary/MasterProblemBinary.hpp"
#include "threshold/MasterProblemThreshold.hpp"

template <typename T>
IterativePricer<T>::IterativePricer(MasterProblemT* mp, SCIP* scip):
    ObjPricer(scip, "IterativePricer", "Iterative pricer for rules", 1, TRUE), mp_(mp)
{
}

template <typename T>
SCIP_RETCODE IterativePricer<T>::scip_init(SCIP* scip, SCIP_PRICER* pricer)
{
    SCIP_CALL(SCIPgetIntParam(scip, "rs/iterative/D", &D));
    SCIP_CALL(SCIPgetIntParam(scip, "rs/iterative/max_candidates", &max_candidates));
    return SCIP_OKAY;
}

template <typename T>
SCIP_RETCODE IterativePricer<T>::scip_redcost(SCIP* scip, SCIP_PRICER* pricer, double* lowerbound,
    unsigned int* stopearly, SCIP_RESULT* result)
{
    const auto lp_obj_val = SCIPgetLPObjval(scip);
    std::cout << "Iterative Pricer Called. Current LP objective: " << lp_obj_val << std::endl;

    MasterProblemT& mp = *mp_;
    ShadowPrices shadow_prices;
    SCIP_CALL(mp.ExtractShadowPrices(scip, shadow_prices));




    auto cmp = [](const std::pair<double, RuleT>& a, const std::pair<double, RuleT>& b) {
        return a.first < b.first; // max-heap on reduced cost
    };
    std::priority_queue<std::pair<double, RuleT>, std::vector<std::pair<double, RuleT>>, decltype(cmp)> heap(cmp);


    for (auto candidate_rules = mp.GetRulesUpToSize(D); const auto& rule : candidate_rules)
    {
        if (const double reduced_cost = mp.ReducedCost(rule, shadow_prices); SCIPisFeasLT(scip, reduced_cost, 0.0))
        {
            if (heap.size() < max_candidates) {
                heap.push({reduced_cost, rule});
            } else if (reduced_cost < heap.top().first) {
                heap.pop();
                heap.push({reduced_cost, rule});
            }
        }
    }

    while (!heap.empty()) {
        auto [red_cost, rule] = heap.top();
        std::cout << "+ Rule: " << rule << "\t Reduced cost: " << red_cost << std::endl;
        uint32_t rule_idx{};
        SCIP_RETCODE(mp.AddRule(scip, rule, true, false, rule_idx));
        heap.pop();
    }

    return SCIP_OKAY;
}

