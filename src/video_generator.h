#pragma once

#include "pch.h"

#include "image_generator.h"

class VideoGenerator
{
public:
    explicit VideoGenerator() {}

    void SetImageGenerator(std::shared_ptr<ImageGenerator> generator) { img_generator_ = generator; };
    void LoadPositions(std::filesystem::path& filename);
    void SetOutputFilePath(std::filesystem::path path);
    void GenerateAndSave();
    void SetBloom(bool bloom) { bloom_ = bloom; }
    void Abort()
    {
        if (rendering_)
            abort_ = true;
    }


private:
    std::shared_ptr<ImageGenerator> img_generator_;
    uint32_t frames_;
    uint32_t framerate_;
    std::vector<glm::vec3> positions_;
    std::vector<glm::vec3> fronts_;
    std::vector<glm::vec3> ups_;
    std::filesystem::path output_path_;
    bool rendering_ = false;
    bool abort_     = false;
    bool bloom_;
};