//
// Created by 20193736 on 18/05/2026.
//

#ifndef BRS_HEURISTICPRICERBINARY_H
#define BRS_HEURISTICPRICERBINARY_H

#include "../MasterProblemBase.hpp"
#include "../HeuristicPricerBase.hpp"
#include "MasterProblemBinary.hpp"

template <>
struct CandidateRuleData<BooleanClause>
{
};

template <>
class HeuristicPricer<BooleanClause> final : public HeuristicPricerBase<BooleanClause>
{
    using MasterProblemB = MasterProblem<BooleanClause>;
    using CandidateRuleB = CandidateRule<BooleanClause>;
    using BooleanRule = Rule<BooleanClause>;

public:
    explicit HeuristicPricer(MasterProblemB* mp, SCIP* scip)
        : HeuristicPricerBase(mp, scip)
    {
    }

    SCIP_RETCODE scip_init(SCIP* scip, SCIP_PRICER* pricer) override;
    SCIP_RETCODE scip_redcost(SCIP* scip, SCIP_PRICER* pricer, SCIP_Real* lowerbound, SCIP_Bool* stopearly,
                              SCIP_RESULT* result) override;
    [[nodiscard]] const roaring::Roaring& PositiveDatapoints(const MasterProblemB& mp) const override;
    [[nodiscard]] const roaring::Roaring& NegativeDatapoints(const MasterProblemB& mp) const override;
    [[nodiscard]] const roaring::Roaring& CoveredDatapoints(const MasterProblemB& mp, uint32_t clause) const override;
    [[nodiscard]] const roaring::Roaring& NonFullyCoveredDatapoints(const MasterProblemB& mp, uint32_t clause) const override;


    BooleanRule ClauseIdxsToRule(const MasterProblemB& mp, const std::set<uint32_t>& clause_idx) const override;
    
    bool KeepClause(SCIP* scip, const MasterProblemB& mp, const ShadowPrices& shadow_prices,
        const std::vector<std::vector<CandidateRule<BooleanClause>>>& layers, const CandidateRuleB& base_candidate,
        size_t depth, uint32_t clause) const override;

    void UpdateEmptyCandidate(const MasterProblemB& mp, const ShadowPrices& shadow_prices,
        CandidateRuleB& empty_candidate) const override;

    void UpdateInferredCandidate(const MasterProblemB& mp, const ShadowPrices& shadow_prices,
        const std::vector<std::vector<CandidateRule<BooleanClause>>>& layers, CandidateRuleB& candidate,
        size_t depth) const override;
};


#endif //BRS_HEURISTICPRICERBINARY_H
