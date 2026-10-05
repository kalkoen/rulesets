//
// Created by 20193736 on 18/05/2026.
//

#ifndef BRS_HEURISTICPRICERTHRESHOLD_H
#define BRS_HEURISTICPRICERTHRESHOLD_H

#include "../MasterProblemBase.hpp"
#include "../HeuristicPricerBase.hpp"
#include "MasterProblemThreshold.hpp"


template <>
struct CandidateRuleData<ThresholdClause>
{
    std::map<uint32_t, double> latest_gammas;
};

template <>
class HeuristicPricer<ThresholdClause> final : public HeuristicPricerBase<ThresholdClause>
{
    using MasterProblemT = MasterProblem<ThresholdClause>;
    using CandidateRuleT = CandidateRule<ThresholdClause>;
    using ThresholdRule = Rule<ThresholdClause>;

    std::vector<double> gamma_flat;

public:
    std::vector<roaring::Roaring> nonzero_datapoints;
    std::vector<roaring::Roaring> nontwo_datapoints;
    uint32_t n_clauses{};
    uint32_t n_datapoints{};

    explicit HeuristicPricer(MasterProblemT* mp, SCIP* scip)
        : HeuristicPricerBase<ThresholdClause>(mp, scip)
    {
    }

    SCIP_RETCODE scip_init(SCIP* scip, SCIP_PRICER* pricer) override;
    SCIP_RETCODE scip_redcost(SCIP* scip, SCIP_PRICER* pricer, SCIP_Real* lowerbound, SCIP_Bool* stopearly,
                              SCIP_RESULT* result) override;

    [[nodiscard]] ThresholdRule ClauseIdxsToRule(const MasterProblemT& mp, const std::set<uint32_t>& clause_idxs) const override;


    [[nodiscard]] const roaring::Roaring& PositiveDatapoints(const MasterProblemT& mp) const override;
    [[nodiscard]] const roaring::Roaring& NegativeDatapoints(const MasterProblemT& mp) const override;

    const roaring::Roaring& CoveredDatapoints(const MasterProblemT& mp, uint32_t clause) const override;
    const roaring::Roaring& NonFullyCoveredDatapoints(const MasterProblemT& mp, uint32_t clause) const override;

    bool KeepClause(SCIP* scip, const MasterProblemT& mp, const ShadowPrices& shadow_prices,
                    const std::vector<std::vector<CandidateRule<ThresholdClause>>>& layers,
                    const CandidateRuleT& base_candidate,
                    size_t depth, uint32_t clause) const override;

    void UpdateEmptyCandidate(const MasterProblemT& mp, const ShadowPrices& shadow_prices,
                              CandidateRuleT& empty_candidate) const override;

    void UpdateInferredCandidate(const MasterProblemT& mp, const ShadowPrices& shadow_prices,
        const std::vector<std::vector<CandidateRule<ThresholdClause>>>& layers, CandidateRuleT& candidate,
        size_t depth) const override;

    [[nodiscard]] double GammaClause(uint32_t datapoint, uint32_t clause) const;


    static double GetLatestGamma(const MasterProblemT& mp,
                                 const std::vector<std::vector<CandidateRule<ThresholdClause>>>& layers,
                                 const CandidateRuleT&
                                 candidate,
                                 uint32_t depth, const ShadowPrices& shadow_prices, uint32_t datapoint);


};


#endif //BRS_HEURISTICPRICERTHRESHOLD_H
