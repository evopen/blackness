#include "pch.h"

#include "video_generator.h"


#include <gtest/gtest.h>


auto filename = std::filesystem::path("/Dev/blackness/resources/position/circular.txt");

TEST(VideoGenerator, PositionLoad)
{
    VideoGenerator v_gen;
    v_gen.LoadPositions(filename);
}