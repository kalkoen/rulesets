//
// Created by 20193736 on 02/06/2026.
//

#include "UpperBoundOracle.hpp"

namespace UpperBoundOracle
{
    std::optional<Cut> FindCutForDatapoint(
        SCIP* scip,
        const SolutionValues& sol,
        uint32_t datapoint,
        const PointData& gamma_data)
    {
        // std::cout << "datapoint: " << datapoint
        //       << " sol.g.size(): " << sol.g.size()
        //       << " sol.z.size(): " << sol.z.size() << std::endl;

        const auto gamma_val = sol.g[datapoint];
        const auto n_clauses = gamma_data.gammas_sorted.size();


        double min_gamma = std::numeric_limits<double>::max();
        for (int l = 0; l < n_clauses; ++l)
        {
            const double gamma_l = gamma_data.gammas_sorted[l];
            if (const uint32_t gamma_index = gamma_data.sorted_index_map[l];
                SCIPisFeasGT(scip, sol.z[gamma_index], 0))
            {
                min_gamma = std::min(min_gamma, gamma_l);
            }
        }

        std::vector<Cut> cuts;
        cuts.reserve(n_clauses);
        if (SCIPisFeasGT(scip, gamma_val, min_gamma))
        {
            // Violated constraint, find a cut
            double violation_cutoff = 0.0;
            for (uint32_t l = n_clauses - 1; l < n_clauses; ++l)
            {
                return FindCut(
                    scip,
                    sol, datapoint,
                    gamma_data,
                    violation_cutoff);
            }
        }
        return std::nullopt;
    }

    std::optional<Cut> FindCut(
        SCIP* scip,
        const SolutionValues& sol,
        const uint32_t datapoint,
        const PointData& gamma_data,
        const double violation_cutoff)
    {
        const auto gamma_sol = sol.g[datapoint];
        const auto n_clauses = gamma_data.gammas_sorted.size();


        Cut c;
        c.datapoint = datapoint;
        c.bound = 2;
        c.violation = gamma_sol - 2;

        std::vector<size_t> sortex_idxs(n_clauses);
        std::iota(sortex_idxs.begin(), sortex_idxs.end(), size_t{0});
        std::ranges::sort(sortex_idxs, std::greater<>{}, [&](size_t l)
        {
            return sol.z[gamma_data.sorted_index_map[l]];
        });

        int n_nonzero = 0;
        for (int l = 0; l < n_clauses; ++l)
        {
            if (SCIPisFeasGT(scip, sol.z[gamma_data.sorted_index_map[l]], 0))
            {
                n_nonzero++;
            }
        }

        c.clause_idxs.reserve(n_nonzero);
        c.clause_coeffs.reserve(n_nonzero);


        size_t sorted_index_cap = n_clauses;
        double last_gamma = 2;

        for (const auto& l_sorted_idx : sortex_idxs)
        {
            const auto gamma_l = gamma_data.gammas_sorted[l_sorted_idx];
            const auto l_real_idx = gamma_data.sorted_index_map[l_sorted_idx];
            const auto z_l = sol.z[l_real_idx];

            if (l_sorted_idx < sorted_index_cap)
            {
                const auto coeff = last_gamma - gamma_l;
                sorted_index_cap = l_sorted_idx;
                c.clause_idxs.push_back(l_real_idx);
                c.clause_coeffs.push_back(coeff);
                c.violation += coeff * z_l;
                last_gamma = gamma_l;
            }
        }

        if (SCIPisFeasGT(scip, c.violation, violation_cutoff))
        {
            // std::cout << "Z sol: ";
            // for (auto z : sol.z)
            // {
            //     std::cout << z << ", ";
            // }
            // std::cout << std::endl;
            //
            // std::cout << "Gamma sol: ";
            // for (auto g : sol.g)
            // {
            //     std::cout << g << ", ";
            // }
            // std::cout << std::endl;
            //
            // for (auto g : gamma_data.gammas_unsorted)
            // {
            //     std::cout << g << ", ";
            // }
            // std::cout << std::endl;
            // for (auto g : gamma_data.gammas_sorted)
            // {
            //     std::cout << g << ", ";
            // }
            // std::cout << std::endl;
            // std::cout << c << std::endl;

            return c;
        }
        return std::nullopt;
    }
}
