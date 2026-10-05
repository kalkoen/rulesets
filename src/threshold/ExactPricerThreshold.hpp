//
// Created by 20193736 on 15/01/2026.
//

#ifndef CODE_EXACTPRICER_H
#define CODE_EXACTPRICER_H

#include "scip/scip.h"
#include "scip/pricer.h"

#include "../MasterProblemBase.hpp"
#include "MasterProblemThreshold.hpp"


#include <vector>
#include <objscip/objpricer.h>

struct SolutionValues
{
    std::vector<double> g;
    std::vector<double> z;

    friend std::ostream& operator<<(std::ostream& os, const SolutionValues& sv)
    {
        os << "SolutionValues{\n g=";
        for (const double i : sv.g)
        {
            os << i << " ";
        }
        os << "\n z=";
        for (const double i : sv.z)
        {
            os << i << " ";
        }
        os << "\n}";
        return os;
    }
};

struct Cut
{
    uint32_t datapoint;
    double violation;
    double bound;
    std::vector<uint32_t> clause_idxs;
    std::vector<double> clause_coeffs;

    friend bool operator>(const Cut& a, const Cut& b)
    {
        return a.violation > b.violation;
    }

    friend std::ostream& operator<<(std::ostream& os, const Cut& c)
    {
        os << "Cut{dp=" << c.datapoint
           << " viol=" << c.violation
           << " bound=" << c.bound
           << " clauses=[";
        for (size_t i = 0; i < c.clause_idxs.size(); ++i)
        {
            if (i > 0) os << ", ";
            os << c.clause_coeffs[i] << "*z_" << c.clause_idxs[i];
        }
        os << "]}";
        return os;
    }
};

enum MinType
{
    LOWER_BOUND = 0,
    UPPER_BOUND= 1
};

struct PointData
{
    std::vector<double> gammas_sorted;
    std::vector<uint32_t> sorted_index_map;
    std::vector<double> gammas_unsorted;
    MinType type;
};

class ExactPricerThreshold : public scip::ObjPricer
{
    using MasterProblemT = MasterProblem<ThresholdClause>;
    using ThresholdRule = Rule<ThresholdClause>;

public:
    MasterProblemT* mp_;
    SCIP* subscip{};
    std::vector<SCIP_VAR*> g;
    std::vector<SCIP_VAR*> z;

    std::vector<PointData> point_data;

    int D{};

    explicit ExactPricerThreshold(MasterProblemT* mp, SCIP* scip) :
        ObjPricer(scip, "ExactPricer_threshold", "Exact pricer for threshold rules", 0, TRUE), mp_(mp)
    {
    }

    SCIP_DECL_PRICERINIT(scip_init) override;
    SCIP_DECL_PRICEREXIT(scip_exit) override;

    SCIP_DECL_PRICERREDCOST(scip_redcost) override;
    SCIP_DECL_PRICERFARKAS(scip_farkas) override;



    [[nodiscard]] SCIP_RETCODE UpdatePricingProblem(const MasterProblemT& mp, const ShadowPrices& shadow_prices) const;

};


#endif //CODE_EXACTPRICER_H
