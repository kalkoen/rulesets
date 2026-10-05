//
// Created by 20193736 on 15/04/2026.
//




#ifndef BRS_THRESHOLDRULE_H
#define BRS_THRESHOLDRULE_H

#include <iostream>
#include <map>
#include <memory>
#include <vector>

struct ThresholdClause
{
    uint32_t feature;
    double threshold;

    [[nodiscard]] nlohmann::json to_json() const
    {
        return { std::to_string(feature) + ">=" + std::to_string(threshold)};
    }

    bool operator<(const ThresholdClause& other) const
    {
        if (feature != other.feature)
            return feature < other.feature;
        return threshold < other.threshold;
    }

    bool operator==(const ThresholdClause& other) const
    {
        return feature == other.feature && threshold == other.threshold;
    }

    friend std::ostream& operator<<(std::ostream& os, const ThresholdClause& c)
    {
        return os << "(" << c.feature << ">=" << c.threshold << ")";
    }

};


#endif //BRS_THRESHOLDRULE_H
