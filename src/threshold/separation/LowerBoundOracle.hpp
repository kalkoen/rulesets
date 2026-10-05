//
// Created by 20193736 on 02/06/2026.
//

#ifndef BRS_POSITIVEORACLE_H
#define BRS_POSITIVEORACLE_H

#include "../ExactPricerThreshold.hpp"
#include "objscip/objscip.h"

#endif //BRS_POSITIVEORACLE_H

namespace LowerBoundOracle
{
    [[nodiscard]] std::optional<Cut> FindCutForDatapoint(
        SCIP* scip,
        const SolutionValues& sol,
        uint32_t datapoint,
        const PointData& point_data,
        double violation_cutoff);
}
