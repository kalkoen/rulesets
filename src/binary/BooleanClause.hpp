//
// Created by 20193736 on 15/04/2026.
//




#ifndef BRS_BOOLEANRULE_H
#define BRS_BOOLEANRULE_H

#include <iostream>
#include <map>
#include <memory>
#include <vector>


struct BooleanClause
{
    uint32_t feature;

    [[nodiscard]] nlohmann::json to_json() const
    {
        return feature;
    }

    bool operator<(const BooleanClause& other) const { return feature < other.feature; }
    bool operator==(const BooleanClause& other) const { return feature == other.feature; }

    friend std::ostream& operator<<(std::ostream& os, const BooleanClause& c)
    {
        return os << c.feature;
    }
};


#endif //BRS_BOOLEANRULE_H
