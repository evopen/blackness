#pragma once

#include "pch.h"

#include "skybox.h"
#include "util.h"

namespace metric::common
{
    cv::Vec3b SkyboxSampler(const glm::dvec3& tex_coord, const Skybox& skybox);
}