//
// Created by 20193736 on 10/04/2026.
//

#ifndef BRS_MasterProblemThreshold_H
#define BRS_MasterProblemThreshold_H

#include <unordered_set>

#include "../MasterProblemBase.hpp"
#include "ThresholdClause.hpp"

#include "roaring.hh"

template <>
class MasterProblemData<ThresholdClause>
{
public:
    std::vector<double> X;

    roaring::Roaring y;
    roaring::Roaring y_inv;

    size_t n_datapoints;
    size_t n_features;

    std::vector<std::string> feature_names;

    std::vector<std::set<uint32_t>> forbidden_feature_combinations;

    static MasterProblemData<ThresholdClause> load_csv(
        const std::string& filenameX,
        const std::string& filenameY, int negation_mode);
};

struct DataNonadditive {
    double eta;
    SCIP_CONS* cons;
};

template <>
class MasterProblem<ThresholdClause> final : public MasterProblemBase<ThresholdClause>
{
    using ThresholdRule = Rule<ThresholdClause>;

    std::vector<ThresholdClause> all_clauses;
    std::vector<std::vector<uint32_t>> clause_idx_by_feature;
    std::vector<double> quantile_levels{};

    SCIP_Bool nonadditive{};
    std::vector<std::map<uint32_t, DataNonadditive>> data_nonadditive;

public:
    const double margin_width;

    MasterProblem(
        MasterProblemData<ThresholdClause> data,
        const int C,
        const double margin_width) :
        MasterProblemBase<ThresholdClause>(std::move(data), 2.0, C), margin_width(margin_width)
    {
        assert(margin_width >= 0);
    }

    SCIP_RETCODE CreateRSModel(SCIP* scip) override;

    [[nodiscard]] size_t NumberOfFeatures() const
    {
        return data.n_features;
    }

    [[nodiscard]] const std::vector<ThresholdClause>& AllClauses() const
    {
        return all_clauses;
    }

    [[nodiscard]] auto ClausesByFeature(const uint32_t feature) const
    {
        return std::views::transform(clause_idx_by_feature[feature], [this](const uint32_t l) -> const ThresholdClause&
       {
           return all_clauses[l];
       });
    }

    [[nodiscard]] std::vector<uint32_t> ClauseIndicesByFeature(const uint32_t feature) const
    {
        return clause_idx_by_feature[feature];
    }

    SCIP_RETCODE AddRule(SCIP *scip, const ThresholdRule &rule, bool asPricerVariable, bool binary, uint32_t& ruleIdx) override;
    SCIP_RETCODE Free(SCIP *scip) override;

    [[nodiscard]] double GammaClause(size_t datapoint, size_t feature, double threshold) const;
    [[nodiscard]] double Gamma(const ThresholdRule& rule, size_t datapoint) const override;
    [[nodiscard]] bool RuleSatisfiesDataPoint(const ThresholdRule& rule, size_t datapoint) const override;
    [[nodiscard]] double ReducedCost(const ThresholdRule& rule, const ShadowPrices& shadow_prices) const override;
    [[nodiscard]] bool IsPositive(size_t datapoint) const override;

    [[nodiscard]] size_t NumberOfDatapoints() const override
    {
        return data.n_datapoints;
    }

    [[nodiscard]] size_t NumberOfClauses() const override
    {
        return all_clauses.size();
    };

    [[nodiscard]] bool SatisfiesThreshold(size_t datapoint, size_t feature, double threshold) const;
    [[nodiscard]] nlohmann::json RuleToJson(const ThresholdRule& rule) const override;
    [[nodiscard]] Generator<Rule<ThresholdClause>> GetRulesUpToSize(int k) override;

    template <typename E>
    [[nodiscard]] std::span<const double> GetVector(const std::vector<E>& flat, const size_t clause) const
    {
        return std::span(&flat[clause * NumberOfDatapoints()], NumberOfDatapoints());
    }

    [[nodiscard]] std::span<const double> Xj(const size_t j) const
    {
        return GetVector(data.X, j);
    }

    SCIP_RETCODE IncludeIterativePricer(SCIP* scip) override;
    SCIP_RETCODE IncludeExactPricer(SCIP* scip) override;
    SCIP_RETCODE IncludeHeuristicPricer(SCIP* scip) override;
    bool IncludeSubRulesInBranchPhase() override;

    std::unique_ptr<MasterProblemBase<ThresholdClause>> Copy() override
    {
        return std::make_unique<MasterProblem<ThresholdClause>>(data, C, margin_width);
    }

    nlohmann::json ToJson(SCIP* scip) override;
};


#endif //BRS_MasterProblemThreshold_H
