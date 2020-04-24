#include "pch.h"

#include "image.h"

#include <gtest/gtest.h>

namespace fs = std::filesystem;

fs::path img_file = "/Dev/blackness/resources/skybox/ChessBoard/texture.png";

TEST(ImageTest, ReinhardToneMapping)
{
    auto hdr_color = glm::vec3(1, 2, 26);
    auto mapped    = Image::ReinhardToneMapping(hdr_color);
    EXPECT_LT(mapped.r, 1.0);
    EXPECT_LT(mapped.g, 1.0);
    EXPECT_LT(mapped.b, 1.0);
}

TEST(ImageTest, LoadImage)
{
    Image image(img_file);
    EXPECT_GT(image.Get(0, 0).r, 0);
    EXPECT_LT(image.Get(0, 0).r, 1);
}