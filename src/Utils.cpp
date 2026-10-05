//
// Created by 20193736 on 02/03/2026.
//

#include <algorithm>
#include <cassert>
#include <charconv>
#include <cmath>
#include <deque>
#include <string>
#include <vector>
#include <map>
#include <ranges>
#include <unordered_set>

#include "Utils.hpp"

namespace Utils
{
    bool is_integer(const std::string_view s)
    {
        int out;
        auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), out);

        // Success means no error AND we consumed the whole string
        return ec == std::errc{} && ptr == s.data() + s.size();
    }

    int integer(const std::string_view s)
    {
        int out;
        auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), out);
        return out;
    }

    bool is_decimal(const std::string_view s)
    {
        double out;
        auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), out);

        // Success means no error AND we consumed the whole string
        return ec == std::errc{} && ptr == s.data() + s.size();
    }

    double decimal(const std::string_view s)
    {
        double out;
        auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), out);
        return out;
    }

    std::vector<int> where(const std::vector<bool>& y, bool is)
    {
        std::vector<int> idxs;
        for (int i = 0; i < y.size(); ++i)
        {
            if (y[i] == is)
            {
                idxs.push_back(i);
            }
        }
        return idxs;
    }

    std::map<std::string, std::string> parse_args(int argc, char* argv[],
                                                  const std::vector<std::string>& loose_arg_order)
    {
        std::map<std::string, std::string> args;
        std::vector<std::string> sources(argv, argv + argc);
        std::deque<std::string> loose_args;

        for (size_t i = 1; i < sources.size(); ++i)
        {
            if (sources[i].starts_with("-"))
            {
                // Strip leading dashes (works for - and --)
                size_t first_alpha = sources[i].find_first_not_of('-');
                std::string key = sources[i].substr(first_alpha);

                // Check if there is a next argument AND it's not another flag
                if (i + 1 < sources.size() && !sources[i + 1].starts_with("-"))
                {
                    args[key] = sources[++i]; // Grab the value and skip it in the next loop
                }
                else
                {
                    args[key] = "true"; // It's just a flag, mark it as true
                }
            }
            else
            {
                // It's a loose argument (no dash)
                loose_args.push_back(sources[i]);
            }
        }

        for (const auto& arg : loose_arg_order)
        {
            if (!loose_args.empty() and !args.contains(arg))
            {
                args[arg] = loose_args.front();
                loose_args.pop_front();
            }
        }

        return args;
    }

    std::vector<size_t> parse_sizes(std::string s)
    {
        std::vector<size_t> widths;
        for (const auto word : s | std::views::split(','))
        {
            std::string_view sv(word.begin(), word.end());
            assert(Utils::is_integer(sv) && "Invalid widths string.");
            widths.push_back(Utils::integer(sv));
        }
        return widths;
    }

    std::vector<double> parse_doubles(std::string_view s)
    {
        std::vector<double> numbers;
        for (const auto word : s | std::views::split(','))
        {
            std::string_view sv(word.begin(), word.end());
            assert(Utils::is_decimal(sv) && "Invalid widths string.");
            numbers.push_back(Utils::decimal(sv));
        }
        return numbers;
    }

    std::vector<double> parse_q(const std::string_view s)
    {
        if (Utils::is_integer(s))
        {
            auto n_quantiles = integer(s);
            assert(n_quantiles > 0);
            std::vector<double> quantiles(static_cast<size_t>(n_quantiles));
            double step = 1.0 / (n_quantiles + 1);
            double i = 0;
            std::ranges::generate(quantiles, [&i, step]()
            {
                return ++i * step;
            });
            return quantiles;
        }
        else
        {
            auto quantiles = parse_doubles(s);
            for (auto q : quantiles)
            {
                assert(0 < q and q < 1);
            }
            return quantiles;
        }
    }

    static Generator<std::unordered_set<uint32_t>> subsets_recurse(
        const std::vector<uint32_t>& vec,
        const std::vector<std::set<uint32_t>>& forbidden,
        std::vector<int>& indices,
        const int n,
        const int ki,
        const int start)
    {
        if (!indices.empty()) {
            std::unordered_set<uint32_t> result;
            result.reserve(indices.size());
            for (auto i : indices) result.insert(vec[i]);
            co_yield result;
        }

        if (static_cast<int>(indices.size()) < ki) {
            for (int i = start; i < n; ++i) {
                bool conflict = false;
                if (!forbidden.empty())
                    for (auto p : indices)
                        if (forbidden[p].contains(vec[i])) { conflict = true; break; }
                if (!conflict)
                {
                    indices.push_back(i);
                    for (auto&& item : subsets_recurse(vec, forbidden, indices, n, ki, i + 1))
                    {
                        co_yield item;
                    }
                    indices.pop_back();
                }
            }
        }
    }

    Generator<std::unordered_set<uint32_t>> subsets_up_to_size(
        const std::vector<uint32_t>& vec,
        const uint32_t max_k,
        const std::vector<std::set<uint32_t>>& forbidden)
    {
        assert(forbidden.size() == 0 or vec.size() == forbidden.size());
        assert(vec.size() <= static_cast<size_t>(std::numeric_limits<int>::max()));
        if (max_k == 0 || vec.empty()) co_return;

        std::vector<int> indices;
        indices.reserve(max_k);
        for (auto&& item : subsets_recurse(vec, forbidden, indices,
                                            static_cast<int>(vec.size()),
                                            static_cast<int>(max_k), 0))
        {
            co_yield item;
        }
    }

    Generator<std::vector<uint32_t>> cartesian_product(
        const std::vector<std::vector<uint32_t>>& vecs,
        const std::unordered_set<uint32_t>& subset)
    {
        if (subset.empty()) { co_yield {}; co_return; }

        // Convert subset to sorted vector for stable iteration order
        std::vector<uint32_t> feat(subset.begin(), subset.end());
        std::ranges::sort(feat);

        if (std::ranges::any_of(feat, [&](uint32_t i) { return vecs[i].empty(); })) co_return;

        const int k = static_cast<int>(feat.size());
        std::vector<int32_t> indices(k, 0);

        while (true) {
            std::vector<uint32_t> result(k);
            for (int i = 0; i < k; ++i)
                result[i] = vecs[feat[i]][indices[i]];
            co_yield result;

            int i = k - 1;
            while (i >= 0 && ++indices[i] == static_cast<int>(vecs[feat[i]].size()))
                indices[i--] = 0;
            if (i < 0) co_return;
        }
    }

    double get_quantile(std::span<const double> data, const double quantile)
    {
        if (data.empty()) return 0.0;
        if (quantile <= 0.0) return *std::ranges::min_element(data);
        if (quantile >= 1.0) return *std::ranges::max_element(data);

        std::vector<double> sorted(data.begin(), data.end());
        std::ranges::sort(sorted);

        const double idx = quantile * (sorted.size() - 1);
        const auto low = static_cast<size_t>(std::floor(idx));
        const auto high = static_cast<size_t>(std::ceil(idx));

        if (low == high) return sorted[low];

        return sorted[low] + (idx - low) * (sorted[high] - sorted[low]);
    }


    std::vector<double> get_quantiles(std::span<const double> data, std::vector<double>& q)
    {
        std::vector<double> values;
        values.reserve(q.size());
        for (const auto qi : q)
            values.push_back(get_quantile(data, qi));

        std::ranges::sort(values);
        values.erase(std::ranges::unique(values).begin(), values.end());
        return values;
    }
}
