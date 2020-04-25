#include "pch.h"

#include "common.h"
#include "flat.h"
#include "util.h"

namespace metric::flat
{
    cv::Vec3b Trace(glm::dvec3 position, glm::dvec3 direction, const Skybox& skybox)
    {
        return metric::common::SkyboxSampler(direction, skybox);
    }
}
