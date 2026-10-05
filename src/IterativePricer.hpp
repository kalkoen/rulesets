//
// Created by 20193736 on 28/06/2026.
//

#ifndef BRS_ITERATIVEPRICERBASE_HPP
#define BRS_ITERATIVEPRICERBASE_HPP

#include "scip/scip.h"
#include "scip/pricer.h"
#include "MasterProblemBase.hpp"
#include "binary/BooleanClause.hpp"
#include "threshold/ThresholdClause.hpp"

template <typename T>
class IterativePricer : public scip::ObjPricer
{
    using MasterProblemT = MasterProblem<T>;
    using RuleT = Rule<T>;

public:
    MasterProblemT* mp_;
    int D{};
    int max_candidates{};

    IterativePricer(MasterProblemT* mp, SCIP* scip);

    SCIP_RETCODE scip_init(SCIP* scip, SCIP_PRICER* pricer) override;

    SCIP_RETCODE scip_redcost(SCIP* scip, SCIP_PRICER* pricer, double* lowerbound, unsigned int* stopearly, SCIP_RESULT* result) override;
};

template class IterativePricer<BooleanClause>;
template class IterativePricer<ThresholdClause>;


#endif //BRS_ITERATIVEPRICERBASE_HPP