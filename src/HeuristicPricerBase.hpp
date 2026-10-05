//
// Created by 20193736 on 15/01/2026.
//

#ifndef CODE_HEURISTICPRICER_B_H
#define CODE_HEURISTICPRICER_B_H

#include "scip/scip.h"
#include "scip/pricer.h"
#include "MasterProblemBase.hpp"

#include <vector>
#include "roaring.hh"
#include <objscip/objpricer.h>

template <typename T>
struct CandidateRuleData;

template <typename T>
struct CandidateRule
{
    std::set<uint32_t> clause_idxs{};
    double reduced_cost{};
    roaring::Roaring inactive_clauses;
    roaring::Roaring pos_covered_datapoints;
    roaring::Roaring neg_covered_datapoints;
    uint32_t parent_clause_idx{};
    uint32_t new_clause_idx{};
    CandidateRuleData<T> custom_data;

    auto operator<=>(const CandidateRule& o) const { return clause_idxs <=> o.clause_idxs; }
    bool operator==(const CandidateRule& o) const { return clause_idxs == o.clause_idxs; }

    [[nodiscard]] uint32_t LowestClauseIdx() const
    {
        return *clause_idxs.begin();
    }
};

template <typename T>
class HeuristicPricer;

template <typename T>
class HeuristicPricerBase : public scip::ObjPricer
{
    using MasterProblemT = MasterProblem<T>;
    using CandidateRuleT = CandidateRule<T>;
    using RuleT = Rule<T>;

public:
    MasterProblemT* mp_;
    std::vector<size_t> beam_widths;
    SCIP_Bool deep{};
    int maximum_nr_candidates{};
    int diversity_maximum{};

    explicit HeuristicPricerBase(MasterProblemT* mp, SCIP* scip);

    virtual const roaring::Roaring& PositiveDatapoints(const MasterProblemT& mp) const = 0;
    virtual const roaring::Roaring& NegativeDatapoints(const MasterProblemT& mp) const = 0;
    virtual const roaring::Roaring& CoveredDatapoints(const MasterProblemT& mp, uint32_t clause) const = 0;
    virtual const roaring::Roaring& NonFullyCoveredDatapoints(const MasterProblemT& mp, uint32_t clause) const = 0;

    virtual bool KeepClause(
        SCIP* scip,
        const MasterProblemT& mp,
        const ShadowPrices& shadow_prices,
        const std::vector<std::vector<CandidateRuleT>>& layers,
        const CandidateRuleT& base_candidate,
        size_t depth,
        uint32_t clause) const = 0;

    virtual void UpdateEmptyCandidate(
        const MasterProblemT& mp,
        const ShadowPrices& shadow_prices,
        CandidateRuleT& empty_candidate
        ) const = 0;

    virtual void UpdateInferredCandidate(
        const MasterProblemT& mp,
        const ShadowPrices& shadow_prices,
        const std::vector<std::vector<CandidateRuleT>>& layers,
        CandidateRuleT& candidate,
        size_t depth
    ) const = 0;

    virtual RuleT ClauseIdxsToRule(const MasterProblemT& mp, const std::set<uint32_t>& clauses) const = 0;

    SCIP_RETCODE Init(SCIP* scip, MasterProblemT& mp);
    SCIP_RETCODE Price(SCIP* scip, MasterProblemT& mp, SCIP_RESULT* result);

    [[nodiscard]] std::vector<RuleT> FindCandidates(SCIP* scip, const MasterProblemT& mp,
                                                const ShadowPrices& shadow_prices) const;

    [[nodiscard]] CandidateRuleT CreateEmptyRuleCandidate(const MasterProblemT& mp,
                                                            const ShadowPrices& shadow_prices) const;

    [[nodiscard]] CandidateRuleT InferNewCandidate(const MasterProblemT& mp,
                                                   const ShadowPrices& shadow_prices, std::vector<std::vector<CandidateRuleT>>& layers,
                                                   int depth, uint32_t clause_idx, uint32_t parent_idx) const;


    void PopulateLayer(SCIP* scip, const MasterProblemT& mp, std::vector<std::vector<CandidateRuleT>>& layers,
                       int depth, size_t width, const ShadowPrices& shadow_prices) const;

    void InferCandidateProperties(const MasterProblemT& mp, std::vector<std::vector<CandidateRuleT>>& layers,
                                  int depth) const;


    template <typename Rule, typename F>
    std::vector<Rule> ChooseDiverse(const std::vector<Rule>& candidates, const size_t num,
                                    F&& get_first_feature) const
    {
        std::vector<Rule> target;
        target.reserve(num);
        std::map<uint32_t, int> diversity_counts;
        size_t j = 0;
        while ((target.size() < num || num == -1) && j < candidates.size())
        {
            const auto& candidate = candidates[j];
            if (diversity_maximum == -1)
            {
                target.push_back(candidate);
            }
            else
            {
                const auto first_feature = get_first_feature(candidate);
                if (diversity_counts[first_feature] < diversity_maximum)
                {
                    target.push_back(std::move(candidate));
                    diversity_counts[first_feature] += 1;
                }
            }
            ++j;
        }
        return target;
    }
};


template <typename Lambda>
void IterateSkipping(const roaring::Roaring& bitmap, const uint32_t range_start, const uint32_t range_end,
                     Lambda&& func)
{
    uint32_t current = range_start;

    // 1. Get an iterator for set bits starting at or after range_start
    auto it = bitmap.begin();
    it.move_equalorlarger(range_start);

    // 2. Enumerate only the "1s" in the bitmap
    while (it != bitmap.end() && *it < range_end)
    {
        uint32_t skip_index = *it;

        // Sub-loop: Invoke lambda for everything BETWEEN the last position and this skip
        for (uint32_t i = current; i < skip_index; ++i)
        {
            func(i);
        }

        // Advance cursor past the skipped bit
        current = skip_index + 1;
        ++it;
    }

    // 3. Final sub-loop: Handle the tail of the range after the last set bit
    for (uint32_t i = current; i < range_end; ++i)
    {
        func(i);
    }
}


#endif //CODE_HEURISTICPRICER_B_H
