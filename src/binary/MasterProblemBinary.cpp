//
// Created by 20193736 on 10/04/2026.
//

#include "MasterProblemBinary.hpp"

#include "../IterativePricer.hpp"
#include "ExactPricerBinary.hpp"
#include "HeuristicPricerBinary.hpp"

#include <objscip/objpricer.h>
#include "../MasterProblemBase.hpp"
#include "../Utils.hpp"

#include "rapidcsv.h"

using BooleanRule = Rule<BooleanClause>;

double MasterProblem<BooleanClause>::Gamma(const BooleanRule& rule, const size_t datapoint) const
{
    return RuleSatisfiesDataPoint(rule, datapoint);
}

bool MasterProblem<BooleanClause>::RuleSatisfiesDataPoint(const BooleanRule& rule, const size_t datapoint) const
{
    return std::ranges::all_of(rule.clauses,
                               [datapoint, this](const BooleanClause cl) { return data.X[cl.feature].contains(datapoint); });
}


double MasterProblem<BooleanClause>::ReducedCost(const BooleanRule& rule, const ShadowPrices& shadow_prices) const
{
    if (rule.empty())
    {
        return DBL_MAX;
    }
    std::vector<const roaring::Roaring*> ptrs;
    for (const BooleanClause cl : rule.clauses)
    {
        ptrs.push_back(&data.X[cl.feature]);
    }
    std::ranges::sort(ptrs, [](const roaring::Roaring* a, const roaring::Roaring* b)
    {
        return a->cardinality() < b->cardinality();
    });
    roaring::Roaring satisfied_datapoints = *ptrs[0];
    for (size_t i = 1; i < ptrs.size(); ++i)
    {
        satisfied_datapoints &= *ptrs[i];
        if (satisfied_datapoints.isEmpty()) break; // Optimization: stop if result is empty
    }
    const uint32_t nr_N = (satisfied_datapoints - data.y).cardinality();
    double sum_contributions_P = 0;
    for (const uint32_t i : satisfied_datapoints & data.y)
    {
        sum_contributions_P += shadow_prices.mu[i];
    }
    return nr_N - sum_contributions_P + shadow_prices.lambda * RuleComplexity(rule);
}

bool MasterProblem<BooleanClause>::IsPositive(size_t datapoint) const
{
    return data.y.contains(datapoint);
}

bool MasterProblem<BooleanClause>::HasFeature(uint32_t datapoint, uint32_t feature) const
{
    return data.X[feature].contains(datapoint);
}

nlohmann::json MasterProblem<BooleanClause>::RuleToJson(const BooleanRule& rule) const
{
    auto j = nlohmann::json::array();
    for (const auto& cl : rule.clauses)
    {
        const auto& name = data.feature_names[cl.feature];
        j.push_back(name);
    }
    return j;
}

Generator<Rule<BooleanClause>> MasterProblem<BooleanClause>::GetRulesUpToSize(int k)
{
    std::vector<uint32_t> indices(NumberOfClauses());
    std::iota(indices.begin(), indices.end(), 0);

    std::vector<std::set<uint32_t>> forbidden(indices.size());

    for (const auto& subset : Utils::subsets_up_to_size(indices, k, forbidden))
    {
        std::set<BooleanClause> clauses(subset.begin(), subset.end());
        co_yield Rule<BooleanClause>(std::move(clauses));
    }
}

SCIP_RETCODE MasterProblem<BooleanClause>::IncludeIterativePricer(SCIP* scip)
{
    auto* iterative_pricer = new IterativePricer<BooleanClause>(this, scip);
    SCIP_CALL(SCIPincludeObjPricer(scip, iterative_pricer, true));
    SCIP_PRICER* pricerPtr = SCIPfindPricer(scip, "IterativePricer");
    assert(pricerPtr != nullptr);
    SCIP_CALL(SCIPactivatePricer(scip, pricerPtr));
    return SCIP_OKAY;
}


SCIP_RETCODE MasterProblem<BooleanClause>::IncludeExactPricer(SCIP* scip)
{
    auto* exact_pricer = new ExactPricerBinary(this, scip);
    SCIP_CALL(SCIPincludeObjPricer(scip, exact_pricer, true));
    SCIP_PRICER* pricerPtr = SCIPfindPricer(scip, "ExactPricer");
    assert(pricerPtr != nullptr);
    SCIP_CALL(SCIPactivatePricer(scip, pricerPtr));
    return SCIP_OKAY;
}

SCIP_RETCODE MasterProblem<BooleanClause>::IncludeHeuristicPricer(SCIP* scip)
{
    auto* heuristic_pricer = new HeuristicPricer<BooleanClause>(this, scip);
    SCIP_CALL(SCIPincludeObjPricer(scip, heuristic_pricer, true));
    SCIP_PRICER* pricerPtr = SCIPfindPricer(scip, "HeuristicPricer");
    assert(pricerPtr != nullptr);
    SCIP_CALL(SCIPactivatePricer(scip, pricerPtr));
    return SCIP_OKAY;
}

bool MasterProblem<BooleanClause>::IncludeSubRulesInBranchPhase()
{
    return false;
}

MasterProblemData<BooleanClause> MasterProblemData<BooleanClause>::load_csv(
    const std::string& filenameX,
    const std::string& filenameY)
{
    MasterProblemData<BooleanClause> mp_data;

    const rapidcsv::Document X_doc(filenameX);
    std::vector<std::string> column_names = X_doc.GetColumnNames();

    const rapidcsv::Document y_doc(filenameY);
    assert(y_doc.GetColumnCount() == 1 && "y file must have a single column");

    auto y_int = y_doc.GetColumn<int>(0);
    std::vector<uint32_t> indices;
    indices.reserve(y_int.size() / 2);
    for (uint32_t i = 0; i < y_int.size(); ++i)
    {
        if (y_int[i] > 0)
        {
            indices.push_back(i);
        }
    }
    mp_data.n_datapoints = y_int.size();
    mp_data.y.addMany(indices.size(), indices.data());
    mp_data.y.runOptimize();

    mp_data.y_inv = mp_data.y;
    mp_data.y_inv.flip(0, y_int.size());

    for (const std::string& header : column_names)
    {
        std::vector<int> x_int = X_doc.GetColumn<int>(header);

        assert(x_int.size() == y_int.size());

        std::vector<uint32_t> indices_x;
        indices_x.reserve(y_int.size() / 2);
        for (uint32_t i = 0; i < x_int.size(); ++i)
        {
            if (x_int[i] > 0)
            {
                indices_x.push_back(i);
            }
        }
        roaring::Roaring x;
        x.addMany(indices_x.size(), indices_x.data());
        x.runOptimize();
        mp_data.X.push_back(x);

        roaring::Roaring x_inv = x;
        x_inv.flip(0, y_int.size());
        x_inv.runOptimize();
        mp_data.X_inv.push_back(x_inv);

        mp_data.feature_names.push_back(header);
    }
    mp_data.n_features = mp_data.X.size();

    return mp_data;
}
