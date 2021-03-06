#include "pch.h"

#include "common.h"
#include "schwarzschild.h"
#include "util.h"

namespace metric::schwarzschild
{
    double GetCosAngle(glm::dvec3 v1, glm::dvec3 v2) { return dot(v1, v2) / (length(v1) * length(v2)); }
    double CalculateImpactParameter(double theta, double r) { return r * std::sin(theta) / std::sqrt(1 - 2 / r); }
    double Geodesic(double r, double b) { return 1 / (r * r * sqrt(1 / (b * b) - 1 / (r * r) + 2 / (r * r * r))); }
    double Geodesic(double r, void* params)
    {
        double b = *(double*) params;
        return Geodesic(r, b);
    }

    double FindClosestApproach(double r0, double b)
    {
        constexpr int kMaxDigits = std::numeric_limits<double>::digits;
        constexpr int kDigits    = kMaxDigits * 3 / 4;

        boost::math::tools::eps_tolerance<double> tol(kDigits);

        std::pair<double, double> result =
            boost::math::tools::bisect([b](double r) { return r / std::sqrt(1 - 2 / r) - b; }, 3.0, r0, tol);

        return (result.first + result.second) / 2 + 1e-7;
    }

    double Integrate(double r0, double r1, double b, gsl_integration_workspace* w)
    {
        gsl_function func;
        func.function = &Geodesic;
        func.params   = &b;

        double dphi, error;

        gsl_integration_qags(&func, r0, r1, 0, 1e-4, 1000, w, &dphi, &error);

        return dphi;
    }


    cv::Vec3b DiskSampler(glm::dvec3 start_pos, double b, double r0, double r1, glm::dvec3 rotation_axis,
        const Blackhole& bh, gsl_integration_workspace* w)
    {
        int direction = 0;
        if (r0 < r1)
        {
            direction = 1;
        }
        else
        {
            direction = -1;
        }
        start_pos = start_pos / length(start_pos) * r0;
        double step_size;
        glm::dvec3 pos_near_disk = start_pos;
        glm::dvec3 pos           = start_pos;

        double r = r0;

        while (true)
        {
            glm::dvec3 last_pos = pos;
            if (abs(pos[1]) < abs(pos_near_disk[1]))
            {
                pos_near_disk = pos;
            }
            step_size = std::max(std::abs(pos.y) * 0.01, 0.001);
            r += direction * step_size;
            if (r * direction > r1 * direction)
                break;
            double dphi = Integrate(r0, r, b, w);
            pos         = glm::rotate(start_pos, abs(dphi), rotation_axis);
            pos         = pos / glm::length(pos) * r;
            if (pos[1] * last_pos[1] < 0)
                break;
        }
        double theta           = std::atan2(pos_near_disk.x, pos_near_disk.z);
        double sampler_x_ratio = (theta + M_PI) / (M_PI * 2);
        double dr              = glm::length(pos_near_disk) - bh.DiskInner();

        int sample_y_index = dr / (bh.DiskOuter() - bh.DiskInner()) * (bh.DiskTexture().size().height - 1);
        int sample_x_index = sampler_x_ratio * (bh.DiskTexture().size().width - 1);

        return bh.DiskTexture().at<cv::Vec3b>(sample_y_index, sample_x_index);
    }

    cv::Vec3b Trace(glm::dvec3 position, glm::dvec3 direction, const Blackhole& bh, const Skybox& skybox,
        gsl_integration_workspace* w, bool* hit)
    {
        glm::dvec3 bh_dir        = bh.Position() - position;
        glm::dvec3 rotation_axis = glm::normalize(glm::cross(direction, bh_dir));
        double cos_theta         = GetCosAngle(direction, bh_dir);
        double theta             = std::acos(cos_theta);
        double r0                = glm::length(position);
        double b                 = CalculateImpactParameter(theta, r0);
        double integrate_end     = 2000;
        if (b < std::sqrt(27))
        {
            double dphi;
            if (r0 < bh.DiskOuter())
                dphi = 0;
            else
                dphi = std::fmod(Integrate(r0, bh.DiskOuter(), b, w), 2 * M_PI);

            glm::dvec3 photon_pos_start = glm::rotate(position, -dphi, rotation_axis);
            double dphi_in_disk         = Integrate(bh.DiskOuter(), bh.DiskInner(), b, w);
            dphi                        = std::fmod(dphi + dphi_in_disk, M_PI * 2);
            glm::dvec3 photon_pos_end   = glm::rotate(position, -dphi, rotation_axis);

            if (std::abs(dphi_in_disk) > M_PI || photon_pos_start[1] * photon_pos_end[1] < 0)
            {
                *hit = true;
                return DiskSampler(photon_pos_start, b, bh.DiskOuter(), bh.DiskInner(), rotation_axis, bh, w);
            }
            return cv::Vec3b(0, 0, 0);
        }
        else
        {
            double r3 = FindClosestApproach(r0, b);
            if (r3 > bh.DiskOuter())
            {
                // Debug
                // return glm::dvec3(0, 0, 1);

                double dphi = std::fmod(Integrate(r0, r3, b, w) - Integrate(r3, integrate_end, b, w), M_PI * 2);

                glm::dvec3 distort_coord = glm::rotate(position, -dphi, rotation_axis);
                return metric::common::SkyboxSampler(distort_coord, skybox);
            }
            else
            {
                double dphi;
                if (r0 < bh.DiskOuter())
                    dphi = 0;
                else
                    dphi = std::fmod(Integrate(r0, bh.DiskOuter(), b, w), 2 * M_PI);
                glm::dvec3 photon_pos_start = glm::rotate(position, -dphi, rotation_axis);

                if (r3 < bh.DiskInner())  // TODO:later
                {
                    double dphi_in_disk       = Integrate(bh.DiskOuter(), bh.DiskInner(), b, w);
                    dphi                      = dphi + dphi_in_disk;
                    glm::dvec3 photon_pos_end = glm::rotate(position, -dphi, rotation_axis);

                    if (std::abs(dphi_in_disk) > M_PI || photon_pos_start[1] * photon_pos_end[1] < 0)
                    {
                        *hit = true;
                        return DiskSampler(photon_pos_start, b, bh.DiskOuter(), bh.DiskInner(), rotation_axis, bh, w);
                    }

                    dphi             = dphi + Integrate(bh.DiskInner(), r3, b, w) - Integrate(r3, bh.DiskInner(), b, w);
                    photon_pos_start = glm::rotate(position, -dphi, rotation_axis);
                    dphi_in_disk     = Integrate(bh.DiskInner(), bh.DiskOuter(), b, w);
                    dphi             = dphi - dphi_in_disk;
                    photon_pos_end   = glm::rotate(position, -dphi, rotation_axis);
                    if (std::abs(dphi_in_disk) > M_PI || photon_pos_start[1] * photon_pos_end[1] < 0)
                    {
                        *hit = true;
                        return DiskSampler(photon_pos_start, b, bh.DiskInner(), bh.DiskOuter(), rotation_axis, bh, w);
                    }

                    dphi                     = dphi - Integrate(bh.DiskOuter(), integrate_end, b, w);
                    glm::dvec3 distort_coord = glm::rotate(position, -dphi, rotation_axis);

                    return metric::common::SkyboxSampler(distort_coord, skybox);
                }
                else
                {
                    double dphi_in_disk       = Integrate(bh.DiskOuter(), r3, b, w);
                    dphi                      = dphi + dphi_in_disk;
                    glm::dvec3 photon_pos_end = glm::rotate(position, -dphi, rotation_axis);

                    if (std::abs(dphi_in_disk) > M_PI || photon_pos_start[1] * photon_pos_end[1] < 0)
                    {
                        *hit = true;
                        return DiskSampler(photon_pos_start, b, bh.DiskOuter(), r3, rotation_axis, bh, w);
                    }
                    photon_pos_start = photon_pos_end;
                    dphi_in_disk     = Integrate(r3, bh.DiskOuter(), b, w);
                    dphi             = std::fmod(dphi - dphi_in_disk, M_PI * 2);
                    photon_pos_end   = glm::rotate(position, -dphi, rotation_axis);
                    if (std::abs(dphi_in_disk) > M_PI || photon_pos_start[1] * photon_pos_end[1] < 0)
                    {
                        *hit = true;
                        return DiskSampler(photon_pos_start, b, r3, bh.DiskOuter(), rotation_axis, bh, w);
                    }

                    // not hit
                    dphi = std::fmod(dphi - Integrate(bh.DiskOuter(), integrate_end, b, w), M_PI * 2);
                    glm::dvec3 distort_coord = glm::rotate(position, -dphi, rotation_axis);

                    return metric::common::SkyboxSampler(distort_coord, skybox);
                }
            }
        }
        return cv::Vec3b(255, 0, 0);
    }
}