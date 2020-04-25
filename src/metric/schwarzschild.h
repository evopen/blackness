#pragma once

#include "pch.h"

#include "blackhole.h"
#include "skybox.h"

namespace metric::schwarzschild
{
    cv::Vec3b Trace(glm::dvec3 position, glm::dvec3 direction, const Blackhole& bh, const Skybox& skybox,
        gsl_integration_workspace* w, bool* hit);
}