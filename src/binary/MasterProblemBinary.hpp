//
// Created by 20193736 on 10/04/2026.
//

#ifndef BRS_MASTERPROBLEMBINARY_B_H
#define BRS_MASTERPROBLEMBINARY_B_H

#include "../MasterProblemBase.hpp"
#include "BooleanClause.hpp"
#include "roaring.hh"

template <>
class MasterProblemData<BooleanClause>
{
public:
    std::vector<roaring::Roaring> X;
    roaring::Roaring y;
    size_t n_datapoints;
    size_t n_features;
    std::vector<std::string> feature_names;

    std::vector<roaring::Roaring> X_inv;
    roaring::Roaring y_inv;


    static MasterProblemData<BooleanClause> load_csv(
        const std::string& filenameX,
        const std::string& filenameY);
};

template<>
class MasterProblem<BooleanClause> final : public MasterProblemBase<BooleanClause> {
public:

    using BooleanRule = Rule<BooleanClause>;

    explicit MasterProblem(
        MasterProblemData<BooleanClause> data,
        const int C) :
    MasterProblemBase(std::move(data), 1.0, C)
    {

    }

    [[nodiscard]] double Gamma(const BooleanRule& rule, size_t datapoint) const override;
    [[nodiscard]] bool RuleSatisfiesDataPoint(const BooleanRule& rule, size_t datapoint) const override;
    [[nodiscard]] double ReducedCost(const BooleanRule& rule, const ShadowPrices& shadow_prices) const override;
    [[nodiscard]] bool IsPositive(size_t datapoint) const override;
    [[nodiscard]] size_t NumberOfDatapoints() const override
    {
        return data.n_datapoints;
    }

    [[nodiscard]] size_t NumberOfClauses() const override
    {
        return data.n_features;
    };

    [[nodiscard]] bool HasFeature(uint32_t datapoint, uint32_t feature) const;
    [[nodiscard]] nlohmann::json RuleToJson(const BooleanRule& rule) const override;
    [[nodiscard]] Generator<Rule<BooleanClause>> GetRulesUpToSize(int k) override;

    SCIP_RETCODE IncludeIterativePricer(SCIP* scip) override;
    SCIP_RETCODE IncludeExactPricer(SCIP* scip) override;
    SCIP_RETCODE IncludeHeuristicPricer(SCIP* scip) override;
    bool IncludeSubRulesInBranchPhase() override;

    std::unique_ptr<MasterProblemBase<BooleanClause>> Copy() override
    {
        return std::make_unique<MasterProblem<BooleanClause>>(data, C);
    }

};


#endif //BRS_MASTERPROBLEMBINARY_B_H
