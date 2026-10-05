//
// Created by 20193736 on 18/05/2026.
//

#include "HeuristicPricerBinary.hpp"

using MasterProblemB = MasterProblem<BooleanClause>;

SCIP_RETCODE HeuristicPricer<BooleanClause>::scip_init(SCIP* scip, SCIP_PRICER* pricer)
{
    MasterProblemB& mp = *mp_;
    return Init(scip, mp);
}

SCIP_RETCODE HeuristicPricer<BooleanClause>::scip_redcost(SCIP* scip, SCIP_PRICER* pricer, SCIP_Real* lowerbound,
                                                        SCIP_Bool* stopearly, SCIP_RESULT* result)
{
    MasterProblemB& mp = *mp_;
    return Price(scip, mp, result);
}

Rule<BooleanClause> HeuristicPricer<BooleanClause>::ClauseIdxsToRule(const MasterProblemB& mp,
                                                        const std::set<uint32_t>& clause_idx) const
{
    BooleanRule rule;
    const std::set<BooleanClause> clauses(clause_idx.begin(), clause_idx.end());
    rule.clauses = clauses;
    return rule;
}

bool HeuristicPricer<BooleanClause>::KeepClause(SCIP* scip, const MasterProblemB& mp, const ShadowPrices& shadow_prices,
                                              const std::vector<std::vector<CandidateRule<BooleanClause>>>& layers,
                                              const CandidateRuleB& base_candidate,
                                              size_t depth, const uint32_t clause) const
{
    // if (SCIPisFeasEQ(scip, shadow_prices.lambda, 0.0))
    // {
    //     return true;
    // }
    // SCIP_Real val = shadow_prices.lambda * (static_cast<double>(base_candidate.size) + 1);
    // for (const auto i : base_candidate.pos_covered_datapoints & mp.data.X[clause])
    // {
    //     val -= shadow_prices.mu[i];
    // }
    // return SCIPisFeasLE(scip, val, 0.0);
    return true;
}

void HeuristicPricer<BooleanClause>::UpdateEmptyCandidate(const MasterProblemB& mp, const ShadowPrices& shadow_prices,
                                                        CandidateRuleB& empty_candidate) const
{
    empty_candidate.reduced_cost += empty_candidate.neg_covered_datapoints.cardinality();
    for (const auto i : empty_candidate.pos_covered_datapoints)
    {
        empty_candidate.reduced_cost -= shadow_prices.mu[i];
    }
}

void HeuristicPricer<BooleanClause>::UpdateInferredCandidate(const MasterProblemB& mp, const ShadowPrices& shadow_prices,
                                                           const std::vector<std::vector<CandidateRule<BooleanClause>>>&
                                                           layers, CandidateRuleB& candidate, size_t depth) const
{
    const auto& base_candidate = layers[depth - 1][candidate.parent_clause_idx];

    for (const auto pos_datapoint :
         base_candidate.pos_covered_datapoints & NonFullyCoveredDatapoints(mp, candidate.new_clause_idx))
    {
        candidate.reduced_cost += shadow_prices.mu[pos_datapoint];
    }

    candidate.reduced_cost -=
        static_cast<double>((base_candidate.neg_covered_datapoints & NonFullyCoveredDatapoints(mp, candidate.new_clause_idx)).
            cardinality());
}

const roaring::Roaring& HeuristicPricer<BooleanClause>::PositiveDatapoints(
    const MasterProblemB& mp) const
{
    return mp.data.y;
}

const roaring::Roaring& HeuristicPricer<BooleanClause>::NegativeDatapoints(
    const MasterProblemB& mp) const
{
    return mp.data.y_inv;
}


const roaring::Roaring& HeuristicPricer<BooleanClause>::CoveredDatapoints(
    const MasterProblemB& mp,
    const uint32_t clause) const
{
    return mp.data.X[clause];
}

const roaring::Roaring& HeuristicPricer<BooleanClause>::NonFullyCoveredDatapoints(const MasterProblemB& mp,
                                                                          const uint32_t clause) const
{
    return mp.data.X_inv[clause];
}
