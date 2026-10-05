//
// Created by 20193736 on 15/01/2026.
//

#ifndef CODE_EXACTPRICER_B_H
#define CODE_EXACTPRICER_B_H

#include "../MasterProblemBase.hpp"
#include "MasterProblemBinary.hpp"

#include "scip/scip.h"
#include "scip/pricer.h"
#include <vector>
#include <objscip/objpricer.h>

class ExactPricerBinary : public scip::ObjPricer
{
    using MasterProblemB = MasterProblem<BooleanClause>;
    using BooleanRule = Rule<BooleanClause>;

public:
    MasterProblemB* mp_;
    SCIP* subscip{};
    std::vector<SCIP_VAR*> z;
    std::vector<SCIP_VAR*> d;
    int D{};
    int constraint_type;

    explicit ExactPricerBinary(MasterProblemB* mp, SCIP* scip) :
        ObjPricer(scip, "ExactPricer", "Exact pricer for rules", 0, TRUE), mp_(mp)
    {

    }

    SCIP_DECL_PRICERINIT(scip_init) override;
    SCIP_DECL_PRICEREXIT(scip_exit) override;

    SCIP_DECL_PRICERREDCOST(scip_redcost) override;
    SCIP_DECL_PRICERFARKAS(scip_farkas) override;

    [[nodiscard]] SCIP_RETCODE UpdatePricingProblem(const MasterProblemB& mp, const ShadowPrices& shadow_prices) const;
};


#endif //CODE_EXACTPRICER_B_H
