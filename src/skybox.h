#pragma once

#include "image.h"

struct Skybox
{
    Image front;
    Image back;
    Image top;
    Image bottom;
    Image left;
    Image right;
};