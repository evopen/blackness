#pragma once

namespace util
{
    template <typename T>
    bool AbsCompare(const T& a, const T& b)
    {
        return (std::abs(a) < std::abs(b));
    }
}