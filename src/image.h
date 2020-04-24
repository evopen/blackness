#pragma once

#include "pch.h"

#include <stb_image.h>
#include <stb_image_write.h>

// HDR image class
class Image
{
public:
    explicit Image() {}
    explicit Image(int width, int height) : width_(width), height_(height)
    {
        img_       = std::shared_ptr<float[]>(new float[width_ * height_ * 4]());
        component_ = 4;
    }
    explicit Image(std::filesystem::path file) { LoadImage(file); }

    void LoadImage(std::filesystem::path file)
    {
        void* data;
        int req_comp = STBI_rgb_alpha;
        data         = stbi_loadf(file.string().c_str(), &width_, &height_, &component_, req_comp);
        if (!data)
        {
            throw std::runtime_error("Failed to load image");
        }

        img_ = std::shared_ptr<float[]>(new float[width_ * height_ * req_comp]());

        memcpy(img_.get(), data, sizeof(float) * width_ * height_ * req_comp);

        stbi_image_free(data);
    }

    inline static glm::vec3 ReinhardToneMapping(glm::vec3 hdr_color)
    {
        float gamma      = 2.2;
        glm::vec3 mapped = hdr_color / (hdr_color + 1.f);
        mapped           = glm::pow(mapped, glm::vec3(1 / gamma));
        return mapped;
    }

    void Save(std::filesystem::path filename)
    {
        if (img_ == nullptr)
            throw std::runtime_error("Image is empty");

        auto img_u8 = std::make_unique<uint8_t[]>(width_ * height_ * component_);

        for (int row = 0; row < height_; ++row)
        {
            for (int col = 0; col < width_; ++col)
            {
                glm::vec4 color  = Get(col, row);
                glm::vec3 mapped = ReinhardToneMapping(glm::vec3(color.r, color.g, color.b));
                img_u8[row * width_ * component_ + col * component_ + 0] = mapped.r * 255.f;
                img_u8[row * width_ * component_ + col * component_ + 1] = mapped.g * 255.f;
                img_u8[row * width_ * component_ + col * component_ + 2] = mapped.b * 255.f;
                img_u8[row * width_ * component_ + col * component_ + 3] = color.a * 255.f;
            }
        }
        stbi_write_png(filename.string().c_str(), width_, height_, component_, img_u8.get(), width_ * component_);
    }

    inline glm::vec4 Get(int x, int y) const
    {

        float* pixel = &img_[y * width_ * component_ + x * component_];
        return glm::vec4(*(pixel + 0), *(pixel + 1), *(pixel + 2), *(pixel + 3));
    }

    inline void Set(int x, int y, glm::vec4 color)
    {

        float* pixel = &img_[y * width_ * component_ + x * component_];
        *(pixel + 0) = color.r;
        *(pixel + 1) = color.g;
        *(pixel + 2) = color.b;
        *(pixel + 3) = color.a;
    }

    inline int Height() const { return height_; }
    inline int Width() const { return width_; }

private:
    std::shared_ptr<float[]> img_;
    int width_;
    int height_;
    int component_;
};