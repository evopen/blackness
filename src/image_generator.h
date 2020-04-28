#pragma once
#include "pch.h"

#include "blackhole.h"
#include "camera.h"
#include "fs.h"
#include "skybox.h"


class ImageGenerator
{
public:
    explicit ImageGenerator() {}
    explicit ImageGenerator(
        std::filesystem::path skybox_folder, Camera cam, uint32_t samples, uint32_t width, uint32_t height)
        : samples_(samples), width_(width), height_(height)
    {
        camera_.reset(new Camera(cam));
        LoadSkybox(skybox_folder);

        color_buffer_.reset(new cv::Mat(height_, width_, CV_8UC3));
    }

    void SetBlackhole(const Blackhole& bh) { blackhole_.reset(new Blackhole(bh)); }
    void RemoveBlackhole() { blackhole_.reset(); }
    void RemoveDisk()
    {
        if (!blackhole_)
        {
            blackhole_->SetDiskInner(1);
            blackhole_->SetDiskOuter(1);
        }
    }

    void LoadSkybox(std::filesystem::path& skybox_folder)
    {
        size_t file_count = fs::CountFiles(skybox_folder);
        if (file_count == 6)
        {
            auto iter = std::filesystem::directory_iterator(skybox_folder);
            for (const auto& file : iter)
            {
                std::string filename = file.path().stem().string();

                if (filename.find("front") != std::string::npos)
                    skybox_.front.reset(new cv::Mat(cv::imread(file.path().string())));
                else if (filename.find("back") != std::string::npos)
                    skybox_.back.reset(new cv::Mat(cv::imread(file.path().string())));
                else if (filename.find("top") != std::string::npos)
                    skybox_.top.reset(new cv::Mat(cv::imread(file.path().string())));
                else if (filename.find("bottom") != std::string::npos)
                    skybox_.bottom.reset(new cv::Mat(cv::imread(file.path().string())));
                else if (filename.find("left") != std::string::npos)
                    skybox_.left.reset(new cv::Mat(cv::imread(file.path().string())));
                else if (filename.find("right") != std::string::npos)
                    skybox_.right.reset(new cv::Mat(cv::imread(file.path().string())));
                else
                    throw std::runtime_error("error loading image");
            }
            cv::flip(*skybox_.front, *skybox_.front, 1);
            cv::rotate(*skybox_.back, *skybox_.back, cv::ROTATE_180);
            cv::rotate(*skybox_.right, *skybox_.right, cv::ROTATE_90_CLOCKWISE);
            cv::rotate(*skybox_.left, *skybox_.left, cv::ROTATE_90_COUNTERCLOCKWISE);
            cv::flip(*skybox_.top, *skybox_.top, 1);
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
    void SetSamples(uint32_t samples) { this->samples_ = samples; }
    void SetCamera(const Camera& cam) { this->camera_.reset(new Camera(cam)); }
    void SetSize(uint32_t width, uint32_t height)
    {
        this->width_  = width;
        this->height_ = height;
        color_buffer_.reset(new cv::Mat(height_, width_, CV_8UC3));
        light_buffer_.reset(new cv::Mat(height_, width_, CV_8UC3));
        *light_buffer_ = 0;
        hdr_buffer_.reset(new cv::Mat(height_, width_, CV_32FC3));
        result_buffer_.reset(new cv::Mat(height_, width_, CV_8UC3));
    }
    void Generate();
    void SaveImageToDisk(std::filesystem::path filename);
    void Abort();
    bool IsRendering() const { return rendering_; }
    void Bloom();
    inline uint32_t Width() const { return width_; }
    inline uint32_t Height() const { return height_; }
    inline std::shared_ptr<Camera> GetCamera() { return camera_; }

    std::shared_ptr<const cv::Mat> ColorBuffer() { return color_buffer_; }
    std::shared_ptr<const cv::Mat> ResultBuffer() { return result_buffer_; }

private:
    std::shared_ptr<Blackhole> blackhole_;
    std::shared_ptr<Camera> camera_;
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
    std::shared_ptr<cv::Mat> hdr_buffer_;
    std::shared_ptr<cv::Mat> result_buffer_;
    bool rendering_ = false;
    bool abort_     = false;
    std::vector<std::thread> thread_pool_;
    bool color_buffer_refreshed_ = true;

    void FlatWorker(int idx);
    void SchwarzschildWorker(int idx);
};