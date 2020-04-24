#pragma once

#include "pch.h"

namespace fs
{
    inline std::size_t CountFiles(std::filesystem::path path)
    {
        using std::filesystem::directory_iterator;
        return std::distance(directory_iterator(path), directory_iterator{});
    }
}