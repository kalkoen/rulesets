//
// Created by 20193736 on 02/03/2026.
//
#pragma once
#define CODE_UTILS_H

#include <string_view>
#include <unordered_set>
#include <coroutine>
#include <optional>


#include "Generator.hpp"
#include "MasterProblemBase.hpp"

namespace Utils
{
    bool is_integer(std::string_view s);
    int integer(std::string_view s);

    bool is_decimal(std::string_view s);
    double decimal(std::string_view s);

    std::vector<int> where(const std::vector<bool>& y, bool is);
    std::map<std::string, std::string> parse_args(int argc, char* argv[],
                                                  const std::vector<std::string>& loose_arg_order);

    Generator<std::unordered_set<uint32_t>> subsets_up_to_size(
        const std::vector<uint32_t>& vec,
        const uint32_t max_k,
        const std::vector<std::set<uint32_t>>& forbidden);

    Generator<std::vector<uint32_t>> cartesian_product(
        const std::vector<std::vector<uint32_t>>& vecs,
        const std::unordered_set<uint32_t>& subset);

    std::vector<size_t> parse_sizes(std::string s);
    std::vector<double> parse_doubles(std::string s);
    std::vector<double> parse_q(std::string_view s);

    // std::vector<uint32_t> computeCompetitionRanks(const std::vector<double>& values);

    double get_quantile(std::span<const double> data, double quantile);
    std::vector<double> get_quantiles(std::span<const double> data, std::vector<double>& q);


    template <typename KeyVec, typename... Vecs>
    void sort_together(KeyVec& key_vec, Vecs&... vecs)
    {
        std::vector<size_t> order(key_vec.size());
        std::iota(order.begin(), order.end(), 0);
        std::ranges::sort(order, std::less<>{}, [&](size_t idx) { return key_vec[idx]; });

        auto apply_permutation = [&order](auto& v)
        {
            std::remove_reference_t<decltype(v)> sorted;
            sorted.reserve(v.size());
            for (size_t idx : order)
                sorted.push_back(std::move(v[idx]));
            v = std::move(sorted);
        };

        apply_permutation(key_vec);
        (apply_permutation(vecs), ...);
    }
}


template <typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& v)
{
    os << "[";
    for (auto it = v.begin(); it != v.end(); ++it)
    {
        os << *it;
        if (std::next(it) != v.end()) os << ", ";
    }
    return os << "]";
}
