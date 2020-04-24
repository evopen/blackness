#pragma once

#include "blackhole.h"
#include "skybox.h"

namespace metric::schwarzschild
{
    glm::dvec3 Trace(glm::dvec3 position, glm::dvec3 direction, const Blackhole& bh, const Skybox& skybox,
        gsl_integration_workspace* w, bool* hit);
}