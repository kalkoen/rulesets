//
// Created by 20193736 on 02/06/2026.
//

#include "LowerBoundOracle.hpp"

namespace LowerBoundOracle
{
    std::optional<Cut> FindCutForDatapoint(SCIP* scip, const SolutionValues& sol, uint32_t datapoint,
                                           const PointData& point_data, double violation_cutoff = 0.0)
    {
        uint32_t l_sorted = 0;
        double total_z = 0.0;
        while (SCIPisFeasLT(scip, total_z, 1.0) && l_sorted < point_data.sorted_index_map.size() - 1)
        {
            const auto unsorted_l = point_data.sorted_index_map[l_sorted];
            total_z += sol.z[unsorted_l];
            l_sorted++;
        }


        const uint32_t k_sorted  = l_sorted;
        const double gamma_k = point_data.gammas_sorted[k_sorted];
        // const uint32_t k_unsorted = gamma_data.sorted_index_map[l_sorted];

        Cut c;

        c.datapoint = datapoint;
        c.bound = gamma_k;

        c.violation =  point_data.gammas_sorted[k_sorted] - sol.g[datapoint];

        // std::cout << "k_sorted " << k_sorted << " gamma_k " << gamma_k << " violation " << c.violation << std::endl;
        //
        // std::cout << "Gamma sorteeeed: ";
        // for (auto tzname : point_data.gammas_sorted)
        // {
        //     std::cout << tzname << " ";
        // }
        // std::cout << std::endl;


        c.clause_coeffs.reserve(k_sorted);
        c.clause_idxs.reserve(k_sorted);
        for (uint32_t i_sorted = 0; i_sorted < k_sorted; i_sorted++)
        {
            const double gamma_i = point_data.gammas_sorted[i_sorted];
            const uint32_t i_unsorted = point_data.sorted_index_map[i_sorted];

            if (SCIPisFeasGT(scip, gamma_k - gamma_i, 0.0))
            {
                c.violation -= (gamma_k - gamma_i)*sol.z[i_unsorted];
                c.clause_coeffs.push_back((gamma_k - gamma_i));
                c.clause_idxs.push_back(i_unsorted);
            }
        }

        if (SCIPisFeasGT(scip, c.violation, violation_cutoff))
        {
            return c;
        } else
        {
            return std::nullopt;
        }
    }
}
