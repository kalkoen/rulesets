//
// Created by 20193736 on 15/01/2026.
//

#include "HeuristicPricerBase.hpp"
#include "MasterProblemBase.hpp"

#include "binary/BooleanClause.hpp"
#include "binary/MasterProblemBinary.hpp"
#include "binary/HeuristicPricerBinary.hpp"

#include "threshold/ThresholdClause.hpp"
#include "threshold/MasterProblemThreshold.hpp"
#include "threshold/HeuristicPricerThreshold.hpp"


#include <algorithm>
#include "scip/scip.h"
#include <cstdio>
#include <format>
#include <ranges>
#include <unordered_set>
#include "typedef.h"
#include "Utils.hpp"

// template <typename T>
// std::vector<uint32_t> CandidateRule<T>::ExtractClauseIdxs(
//     const std::vector<std::vector<CandidateRule>>& layers,
//     size_t layer) const
// {
//     std::vector<uint32_t> clauses;
//     if (layer == 0)
//     {
//         return clauses;
//     }
//
//     size_t current_parent = parent_idx;
//     clauses.push_back(new_clause);
//     while (layer > 1)
//     {
//         const auto& current = layers[layer - 1][current_parent];
//         clauses.push_back(current.new_clause);
//         current_parent = current.parent_idx;
//         --layer;
//     }
//
//     std::ranges::sort(clauses);
//     return clauses;
// }

template <typename T>
HeuristicPricerBase<T>::HeuristicPricerBase(MasterProblemT* mp, SCIP* scip):
    ObjPricer(scip, "HeuristicPricer", "Heuristic pricer for rules", 1, TRUE), mp_(mp)
{
}

template <typename T>
SCIP_RETCODE HeuristicPricerBase<T>::Init(SCIP* scip, MasterProblemT& mp)
{
    beam_widths.clear();
    char* beam_widths_cstr;
    SCIP_CALL(SCIPgetStringParam(scip, "rs/heuristic/beam_widths", &beam_widths_cstr));
    const std::string beam_widths_str(beam_widths_cstr);
    beam_widths = Utils::parse_sizes(beam_widths_str);

    deep = false;

    SCIP_CALL(SCIPgetIntParam(scip, "rs/heuristic/max_candidates", &maximum_nr_candidates));
    SCIP_CALL(SCIPgetIntParam(scip, "rs/heuristic/diversity", &diversity_maximum));
    SCIP_CALL(SCIPgetBoolParam(scip, "rs/heuristic/deep", &deep));


    return SCIP_OKAY;
}


template <typename T>
SCIP_RETCODE HeuristicPricerBase<T>::Price(
    SCIP* scip,
    MasterProblemT& mp,
    SCIP_RESULT* result)
{
    *result = SCIP_SUCCESS;

    // if (SCIPgetLPSolstat(scip) != SCIP_LPSOLSTAT_OPTIMAL)
    // {
    //     std::cout << "Heuristic Pricer: LP not optimal, skipping." << std::endl;
    //     return SCIP_OKAY;
    // }



    ShadowPrices shadow_prices;
    SCIP_CALL(mp.ExtractShadowPrices(scip, shadow_prices));

    const auto lp_obj_val = SCIPgetLPObjval(scip);
    std::cout << "Heuristic Pricer Called. Current LP objective: " << lp_obj_val << " Lambda " << shadow_prices.lambda << std::endl;

    auto candidates = FindCandidates(scip, mp, shadow_prices);

    auto chosen = ChooseDiverse(candidates,
                                maximum_nr_candidates,
                                [](const RuleT& c)
                                      {
                                          return c.clauses.begin()->feature;
                                      });


    for (auto& rule : chosen)
    {
        uint32_t rule_idx{};
        SCIP_CALL(mp.AddRule(scip, rule, true, false, rule_idx));
        auto red_cost = mp.ReducedCost(rule, shadow_prices);
        std::cout << "+ Rule: " << rule << "\t Reduced cost: " << red_cost << std::endl;
    }

    if (!chosen.empty())
    {
        const auto& best = chosen[0];
        const auto red_cost = mp.ReducedCost(best, shadow_prices);
        mp.RecordPricingStep(scip, PRICER_HEURISTIC, red_cost);
    } else
    {
        std::cout << "No rule found." << std::endl;
    }

    return SCIP_OKAY;
}

template <typename T>
std::vector<Rule<T>> HeuristicPricerBase<T>::FindCandidates(SCIP* scip, const MasterProblemT& mp,
                                                      const ShadowPrices& shadow_prices) const
{
    std::vector<std::vector<CandidateRule<T>>> layers(beam_widths.size() + 1);

    layers[0].push_back(CreateEmptyRuleCandidate(mp, shadow_prices));

    int i = 1;
    for (const auto width : beam_widths)
    {
        PopulateLayer(
            scip,
            mp,
            layers,
            i,
            width == 0 ? std::numeric_limits<size_t>::max() : width,
            shadow_prices);
        ++i;
    }

    // Free memory.
    for (auto& layer : layers)
    {
        for (auto& candidate : layer)
        {
            candidate.inactive_clauses = {};
            candidate.pos_covered_datapoints = {};
            candidate.neg_covered_datapoints = {};
        }
    }

    size_t total = 0;
    for (const auto& v : layers) total += v.size();
    std::vector<RuleT> candidates;
    candidates.reserve(total);

    std::vector<double> reduced_costs;
    reduced_costs.reserve(total);

    for (size_t layer_idx = 1; layer_idx < layers.size(); ++layer_idx)
    {
        auto& layer = layers[layer_idx];

        for (std::vector<CandidateRule<T>>& typed_layer = layer; auto& candidate : typed_layer)
        {
            if (SCIPisFeasLT(scip, candidate.reduced_cost, 0.0))
            {
                candidates.push_back(ClauseIdxsToRule(mp, candidate.clause_idxs));
                reduced_costs.push_back(candidate.reduced_cost);
                // std::cout << candidate.ExtractClauses(layers, layer_idx) << " "  << candidate.reduced_cost << std::endl;
            }
        }
    }

    Utils::sort_together(reduced_costs, candidates);

    return candidates;
}

template <typename T>
CandidateRule<T> HeuristicPricerBase<T>::CreateEmptyRuleCandidate(const MasterProblemT& mp,
                                                                  const ShadowPrices& shadow_prices) const
{
    CandidateRule<T> empty_rule;

    empty_rule.pos_covered_datapoints = PositiveDatapoints(mp);
    empty_rule.neg_covered_datapoints = NegativeDatapoints(mp);

    if (mp_->RuleComplexityType() == COMPLEXITY_ONE or mp_->RuleComplexityType() == COMPLEXITY_ONE_PLUS_CLAUSES)
    {
        empty_rule.reduced_cost = shadow_prices.lambda;
    } else
    {
        empty_rule.reduced_cost = 0.0;
    }

    UpdateEmptyCandidate(mp, shadow_prices, empty_rule);

    empty_rule.parent_clause_idx = std::numeric_limits<uint32_t>::max();
    empty_rule.new_clause_idx = std::numeric_limits<uint32_t>::max();

    return empty_rule;
}

template <typename T>
void HeuristicPricerBase<T>::PopulateLayer(SCIP* scip, const MasterProblemT& mp,
                                           std::vector<std::vector<CandidateRule<T>>>& layers,
                                           int depth, const size_t width,
                                           const ShadowPrices& shadow_prices) const
{
    std::vector<CandidateRule<T>> layer_candidates;

    size_t reserve = layers[depth-1].size() * mp.NumberOfClauses();
    for (size_t base_candidate_idx = 0; base_candidate_idx < layers[depth - 1].size(); ++base_candidate_idx)
    {
        auto& base_candidate = layers[depth - 1][base_candidate_idx];
        reserve -= base_candidate.inactive_clauses.cardinality();
    }
    layer_candidates.reserve(reserve);

    for (size_t base_candidate_idx = 0; base_candidate_idx < layers[depth - 1].size(); ++base_candidate_idx)
    {
        auto& base_candidate = layers[depth - 1][base_candidate_idx];

        auto& base_candidate_features = base_candidate.clause_idxs;

        std::vector<uint32_t> deactivate;
        std::vector<CandidateRule<T>> children;
        IterateSkipping(base_candidate.inactive_clauses, 0, mp.NumberOfClauses(), [&](const uint32_t clause)
        {

            // std::cout << "For new clause " << base_candidate.new_clause << " trying " << clause << std::endl;

            if (KeepClause(scip, mp, shadow_prices, layers, base_candidate, depth, clause))
            {
                const auto it = std::ranges::lower_bound(base_candidate_features, clause);

                base_candidate_features.insert(it, clause);

                CandidateRule<T> new_candidate = InferNewCandidate(
                    mp,
                    shadow_prices,
                    layers,
                    depth,
                    clause,
                    base_candidate_idx);
                children.push_back(std::move(new_candidate));

                base_candidate_features.erase(std::ranges::lower_bound(base_candidate_features, clause));
            }
            else
            {
                deactivate.push_back(clause);
            }
        });
        for (const uint32_t feat : deactivate)
        {
            base_candidate.inactive_clauses.add(feat);
        }
        base_candidate.inactive_clauses.runOptimize();

        size_t n_to_save = children.size();
        if (deep)
        {
            // Deep search, so we save the top-width choices. And later, we don't prune anymore.
            std::ranges::sort(children, std::less<>{}, &CandidateRule<T>::reduced_cost);
            n_to_save = std::min(width, children.size());
        }

        auto slice = std::span(children.data(), n_to_save);
        layer_candidates.insert(layer_candidates.end(), slice.begin(), slice.end());
    }

    std::ranges::sort(layer_candidates);
    auto dup = std::ranges::unique(layer_candidates);
    layer_candidates.erase(dup.begin(), dup.end());
    std::ranges::sort(layer_candidates, std::less<>{}, &CandidateRule<T>::reduced_cost);

    // Sort by increasing reduced cost
    std::ranges::sort(layer_candidates, std::less<>{}, &CandidateRule<T>::reduced_cost);

    if (not deep)
    {
        // Beam search. We added all the candidates earlier. Now we pick the top-width, keeping in mind diversity constraints
        layers[depth] = ChooseDiverse(layer_candidates,
                                      width,
                                      [](const CandidateRule<T>& c)
                                      {
                                          return c.LowestClauseIdx();
                                      });
    }
    else
    {
        layers[depth] = std::move(layer_candidates);
    }
    InferCandidateProperties(mp, layers, depth);
}

template <typename T>
void HeuristicPricerBase<T>::InferCandidateProperties(const MasterProblemT& mp,
                                                      std::vector<std::vector<CandidateRule<T>>>& layers,
                                                      const int depth) const
{
    for (auto& candidate : layers[depth])
    {
        const auto clause = candidate.new_clause_idx;
        const auto& parent = layers[depth - 1][candidate.parent_clause_idx];

        candidate.pos_covered_datapoints =
            parent.pos_covered_datapoints & CoveredDatapoints(mp, clause);
        candidate.pos_covered_datapoints.runOptimize();

        candidate.neg_covered_datapoints =
            parent.neg_covered_datapoints & CoveredDatapoints(mp, clause);
        candidate.neg_covered_datapoints.runOptimize();

        candidate.inactive_clauses |= parent.inactive_clauses;
        candidate.inactive_clauses.add(candidate.new_clause_idx);
        candidate.inactive_clauses.runOptimize();
    }
}

template <typename T>
CandidateRule<T> HeuristicPricerBase<T>::InferNewCandidate(const MasterProblemT& mp,
                                                           const ShadowPrices& shadow_prices,
                                                           std::vector<std::vector<CandidateRule<T>>>& layers,
                                                           const int depth,
                                                           const uint32_t clause_idx,
                                                           const uint32_t parent_idx) const
{
    CandidateRule<T> candidate;

    const auto& base_candidate = layers[depth - 1][parent_idx];

    candidate.clause_idxs = base_candidate.clause_idxs;
    candidate.clause_idxs.insert(clause_idx);
    candidate.new_clause_idx = clause_idx;
    candidate.parent_clause_idx = parent_idx;

    candidate.reduced_cost = base_candidate.reduced_cost;
    if (mp_->RuleComplexityType() == COMPLEXITY_CLAUSES or mp_->RuleComplexityType() == COMPLEXITY_ONE_PLUS_CLAUSES)
    {
        candidate.reduced_cost+= shadow_prices.lambda;
    }

    UpdateInferredCandidate(mp, shadow_prices, layers, candidate, depth);

    return candidate;
}

template class HeuristicPricerBase<BooleanClause>;
template class HeuristicPricerBase<ThresholdClause>;
