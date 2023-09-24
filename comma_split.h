#pragma once

#include <string>
#include <vector>
#include <sstream>

std::vector<std::string> split_comma(const std::string& str) {
    std::vector<std::string> result;
    std::stringstream sstream(str);
    std::string item;

    while (std::getline(sstream, item, ',')) {
        result.push_back(item);
    }

    return result;
}

template<typename T>
std::vector<T> concat(const std::vector<T>& lhs, const std::vector<T>& rhs) {
    std::vector<T> result;
    
    // Reserve space for both vectors to avoid unnecessary allocations
    result.reserve(lhs.size() + rhs.size());

    // Append the elements of lhs to the result
    result.insert(result.end(), lhs.begin(), lhs.end());

    // Append the elements of rhs to the result
    result.insert(result.end(), rhs.begin(), rhs.end());

    return result;
}
