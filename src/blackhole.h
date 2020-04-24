#pragma once
#include "pch.h"

#include "image.h"

class Blackhole
{

public:
    explicit Blackhole(glm::dvec3 position) : position_(position), has_disk_(false) {}

    explicit Blackhole(glm::dvec3 position, double disk_inner, double disk_outer, std::filesystem::path texture_path)
        : position_(position), disk_inner_(disk_inner), disk_outer_(disk_outer), has_disk_(true)
    {
        disk_texture_.LoadImage(texture_path);
    }

    double DiskInner()
    {
        if (has_disk_)
        {
            return disk_inner_;
        }
        else
        {
            throw std::runtime_error("Blackhole has no accretion disk");
        }
    }

    double DiskOuter()
    {
        if (has_disk_)
        {
            return disk_outer_;
        }
        else
        {
            throw std::runtime_error("Blackhole has no accretion disk");
        }
    }

    inline bool HasDisk() { return has_disk_; }

    inline glm::dvec3 Position() const { return position_; }
    inline double DiskOuter() const { return disk_outer_; }
    inline double DiskInner() const { return disk_inner_; }
    inline Image DiskTexture() const { return disk_texture_; }

private:
    glm::dvec3 position_;
    double disk_inner_;
    double disk_outer_;
    bool has_disk_;
    Image disk_texture_;
};