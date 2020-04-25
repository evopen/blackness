#pragma once

#include "pch.h"

#include "skybox.h"

namespace metric::flat
{
    cv::Vec3b Trace(glm::dvec3 position, glm::dvec3 direction, const Skybox& skybox);
}