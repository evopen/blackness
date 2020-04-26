#pragma once
#include "pch.h"


class Blackhole
{

public:
    explicit Blackhole(glm::dvec3 position) : position_(position), has_disk_(false) {}

    explicit Blackhole(glm::dvec3 position, double disk_inner, double disk_outer, std::filesystem::path texture_path)
        : position_(position), disk_inner_(disk_inner), disk_outer_(disk_outer), has_disk_(true)
    {
        disk_texture_.reset(new cv::Mat(cv::imread(texture_path.string())));
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

    void SetDiskInner(double radius) { disk_inner_ = radius; }
    void SetDiskOuter(double radius) { disk_outer_ = radius; }

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
    inline const cv::Mat& DiskTexture() const { return *disk_texture_; }

private:
    glm::dvec3 position_;
    double disk_inner_;
    double disk_outer_;
    bool has_disk_;
    std::shared_ptr<cv::Mat> disk_texture_;
};