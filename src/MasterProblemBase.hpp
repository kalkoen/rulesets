//
// Created by 20193736 on 15/01/2026.
//

#ifndef CODE_MASTERPROBLEM_H
#define CODE_MASTERPROBLEM_H

#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <vector>
#include <objscip/objprobdata.h>
#include <objscip/objpricer.h>
#include <scip/def.h>
#include <scip/type_cons.h>
#include <scip/type_var.h>

#include "Generator.hpp"
#include "json.hpp"

struct ShadowPrices
{
    SCIP_Real lambda;
    std::vector<SCIP_Real> mu;
};

struct PricingStepData
{
    int pricer;
    SCIP_Real lp_objective;
    SCIP_Real lowest_red_cost;
    double time;
};

struct ColumnGenerationMetrics
{
    std::vector<PricingStepData> pricing_steps;
    SCIP_Real relaxation_value;
    SCIP_Real soltime_cg;
    SCIP_STATUS status_cg;
};

template <typename Clause>
struct Rule
{
    std::set<Clause> clauses;

    int size() const
    {
        return clauses.size();
    }

    bool empty() const
    {
        return clauses.empty();
    }

    bool operator<(const Rule& other) const
    {
        return clauses < other.clauses;
    }

    bool operator==(const Rule& other) const
    {
        return clauses == other.clauses;
    }

    [[nodiscard]] nlohmann::json to_json() const
    {
        nlohmann::json arr = nlohmann::json::array();
        for (const auto& clause : clauses)
            arr.push_back(clause.to_json());
        return arr;
    }

    friend std::ostream& operator<<(std::ostream& os, const Rule& rule)
    {
        os << "{ ";
        for (const auto& clause : rule.clauses)
            os << clause << " ";
        os << "}";
        return os;
    }
};

template <typename T>
concept RSClause = requires(T r, T other)
{
    // 2. Must be comparable for std::set or std::map (your rules.insert(rule) call)
    { r < other } -> std::convertible_to<bool>;
    { r == other } -> std::convertible_to<bool>;

    { r.to_json() } -> std::convertible_to<nlohmann::json>;

    { std::cout << r } -> std::same_as<std::ostream&>;
};

template <typename RSClause>
class MasterProblemData
{
    MasterProblemData() = default;
};


template <typename T>
class MasterProblem;

template <typename RSClause>
class MasterProblemBase
{
    using RSRule = Rule<RSClause>;

protected:
    std::map<size_t, SCIP_VAR*> zeta;
    std::vector<SCIP_VAR*> w;

    std::vector<RSRule> w_rules;
    std::set<RSRule> rules;
    ColumnGenerationMetrics cg_metrics = {};

    std::map<size_t, SCIP_CONS*> fn_constraints;
    SCIP_CONS* complexity_constraint{};

    int rule_complexity_type{};
    SCIP_Bool cover_positive;

public:
    virtual ~MasterProblemBase() = default;

    const MasterProblemData<RSClause> data;

    const double max_zeta;
    const int C;


    explicit MasterProblemBase(
        MasterProblemData<RSClause> data,
        const double max_zeta,
        const int C) : data(std::move(data)), max_zeta(max_zeta), C(C)
    {
    }

    [[nodiscard]] int RuleComplexityType() const
    {
        return rule_complexity_type;
    };

    double RuleComplexity(const RSRule& rule) const;

    [[nodiscard]] virtual double Gamma(const RSRule& rule, size_t datapoint) const = 0;
    [[nodiscard]] virtual bool RuleSatisfiesDataPoint(const RSRule& rule, size_t datapoint) const = 0;
    [[nodiscard]] virtual double ReducedCost(const RSRule& rule, const ShadowPrices& shadow_prices) const = 0;
    [[nodiscard]] virtual bool IsPositive(size_t datapoint) const = 0;

    [[nodiscard]] virtual size_t NumberOfClauses() const = 0;
    [[nodiscard]] virtual size_t NumberOfDatapoints() const = 0;
    [[nodiscard]] virtual nlohmann::json RuleToJson(const RSRule& rule) const = 0;
    [[nodiscard]] virtual Generator<Rule<RSClause>> GetRulesUpToSize(int k) = 0;
    [[nodiscard]] virtual std::unique_ptr<MasterProblemBase> Copy() = 0;

    virtual SCIP_RETCODE IncludeIterativePricer(SCIP* scip) = 0;
    virtual SCIP_RETCODE IncludeExactPricer(SCIP* scip) = 0;
    virtual SCIP_RETCODE IncludeHeuristicPricer(SCIP* scip) = 0;

    virtual bool IncludeSubRulesInBranchPhase() = 0;

    virtual SCIP_RETCODE CreateRSModel(
        SCIP* scip);

    virtual SCIP_RETCODE AddRule(SCIP* scip, const RSRule& rule, bool asPricerVariable, bool binary, uint32_t& ruleIdx);
    virtual SCIP_RETCODE AddRuleAndSubRules(
        SCIP* scip,
        const RSRule& rule,
        bool asPricerVariable,
        bool binary);

    virtual SCIP_RETCODE Free(SCIP* scip);

    SCIP_RETCODE Solve(SCIP* scip);

    virtual nlohmann::json ToJson(SCIP* scip);

    SCIP_RETCODE RecordPricingStep(SCIP* scip, int pricer, double lowest_red_cost);

    SCIP_RETCODE ExtractShadowPrices(SCIP* scip, ShadowPrices& shadow_prices) const;

    static double ComputeCGTimeLimit(SCIP* scip, const std::string& timelimit_setting);
};

#endif //CODE_MASTERPROBLEM_H
