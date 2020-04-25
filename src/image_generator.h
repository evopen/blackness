#pragma once
#include "pch.h"

#include "blackhole.h"
#include "camera.h"
#include "fs.h"
#include "skybox.h"


class ImageGenerator
{
public:
    explicit ImageGenerator(
        std::filesystem::path skybox_folder, Camera cam, uint32_t samples, uint32_t width, uint32_t height)
        : samples_(samples), width_(width), height_(height), camera_(cam)
    {
        LoadSkybox(skybox_folder);
    }

    void SetBlackhole(const Blackhole& bh) { blackhole_.reset(new Blackhole(bh)); }

    void LoadSkybox(std::filesystem::path& skybox_folder)
    {
        size_t file_count = fs::CountFiles(skybox_folder);
        if (file_count == 6)
        {
            auto iter = std::filesystem::directory_iterator(skybox_folder);
            for (const auto& file : iter)
            {
                std::string filename = file.path().stem().string();

                if (filename == "front")
                    skybox_.front.reset(new cv::Mat(cv::imread(file.path().string())));
                else if (filename == "back")
                    skybox_.back.reset(new cv::Mat(cv::imread(file.path().string())));
                else if (filename == "top")
                    skybox_.top.reset(new cv::Mat(cv::imread(file.path().string())));
                else if (filename == "bottom")
                    skybox_.bottom.reset(new cv::Mat(cv::imread(file.path().string())));
                else if (filename == "left")
                    skybox_.left.reset(new cv::Mat(cv::imread(file.path().string())));
                else if (filename == "right")
                    skybox_.right.reset(new cv::Mat(cv::imread(file.path().string())));
                else
                    throw std::runtime_error("error loading image");
            }
        }
        else if (file_count == 1)
        {
            auto iter = std::filesystem::directory_iterator(skybox_folder);
            skybox_.front.reset(new cv::Mat(cv::imread(iter->path().string())));
            skybox_.left = skybox_.right = skybox_.bottom = skybox_.top = skybox_.back = skybox_.front;
        }
        else
        {
            throw std::runtime_error("Failed to find skybox texture files");
        }
    }

    void SetThreads(uint32_t threads) { this->threads_ = threads; }
    void Generate();
    void SaveImageToDisk(std::filesystem::path filename);
    std::shared_ptr<const cv::Mat> ResultImage() { return color_buffer_; }

private:
    std::shared_ptr<Blackhole> blackhole_;
    Camera camera_;
    Skybox skybox_;
    uint32_t samples_;
    uint32_t width_;
    uint32_t height_;
    bool hdr_;
    bool bloom_;
    uint32_t threads_ = 1;
    std::mt19937 rng_;
    std::shared_ptr<cv::Mat> color_buffer_;
    std::shared_ptr<cv::Mat> light_buffer_;

    void FlatWorker(int idx);
    void SchwarzschildWorker(int idx);
};