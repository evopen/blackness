#include "pch.h"

#include "image_generator.h"
#include "metric/flat.h"
#include "metric/schwarzschild.h"



void ImageGenerator::Generate()
{
    abort_     = false;
    rendering_ = true;
    if (!blackhole_)
    {
        for (int i = 0; i < threads_; ++i)
        {
            thread_pool_.emplace_back(std::thread(&ImageGenerator::FlatWorker, this, i));
        }
    }
    else
    {
        for (int i = 0; i < threads_; ++i)
        {
            thread_pool_.emplace_back(std::thread(&ImageGenerator::SchwarzschildWorker, this, i));
        }
    }
    for (int i = 0; i < threads_; ++i)
    {
        thread_pool_[i].join();
    }
    thread_pool_.clear();
    rendering_ = false;
    color_buffer_refreshed_ = true;
}

void ImageGenerator::Abort()
{
    if (rendering_)
    {
        abort_ = true;
    }
}

void ImageGenerator::SchwarzschildWorker(int idx)
{
    std::uniform_real_distribution<double> uni(-0.001, 0.001);
    gsl_integration_workspace* workspace = gsl_integration_workspace_alloc(1000);
    for (int row = idx; row < height_; row += threads_)
    {
        for (int col = 0; col < width_; ++col)
        {
            if (abort_)
            {
                return;
            }
            glm::dvec3 tex_coord = camera_->GetTexCoord(row, col, width_, height_);
            cv::Vec3f color(0, 0, 0);
            bool hit = false;

            for (int sample = 0; sample < samples_; sample++)
            {
                glm::dvec3 sample_coord;
                if (samples_ != 1)
                    sample_coord = tex_coord + glm::dvec3(uni(rng_), uni(rng_), uni(rng_));
                else
                    sample_coord = tex_coord;
                color += (cv::Vec3f) metric::schwarzschild::Trace(
                             camera_->Position(), sample_coord, *blackhole_, skybox_, workspace, &hit)
                         / float(samples_);
            }
            color_buffer_->at<cv::Vec3b>(row, col) = color;

            if (hit)
                light_buffer_->at<cv::Vec3b>(row, col) = color;
        }
    }
}


void ImageGenerator::FlatWorker(int idx)
{
    std::uniform_real_distribution<double> uni(-0.001, 0.001);
    for (int row = idx; row < height_; row += threads_)
    {
        for (int col = 0; col < width_; ++col)
        {
            if (abort_)
            {
                return;
            }
            glm::dvec3 tex_coord = camera_->GetTexCoord(row, col, width_, height_);
            cv::Vec3d color(0, 0, 0);

            for (int sample = 0; sample < samples_; sample++)
            {
                glm::dvec3 sample_coord;
                if (samples_ != 1)
                    sample_coord = tex_coord + glm::dvec3(uni(rng_), uni(rng_), uni(rng_));
                else
                    sample_coord = tex_coord;
                color += (cv::Vec3f) metric::flat::Trace(camera_->Position(), sample_coord, skybox_) / float(samples_);
            }
            color_buffer_->at<cv::Vec3b>(row, col) = color;
        }
    }
}

void ImageGenerator::SaveImageToDisk(std::filesystem::path filename)
{
    cv::imwrite(filename.string(), *color_buffer_);
}


void ImageGenerator::Bloom()
{
    if (!color_buffer_refreshed_)
        return;
    cv::Mat bloom_buffer = cv::Mat::zeros(width_, height_, CV_8UC3);
    cv::GaussianBlur(*light_buffer_, bloom_buffer, cv::Size(15, 15), 0);
    cv::add(*color_buffer_, bloom_buffer, *hdr_buffer_, cv::noArray(), CV_32FC3);

    auto tonemap = cv::createTonemapReinhard(1.f);
    cv::Mat ldr;

    tonemap->process(*hdr_buffer_, ldr);
    ldr *= 255;
    result_buffer_.reset(new cv::Mat(width_, height_, CV_8UC3));
    ldr.convertTo(*result_buffer_, CV_8UC3);
    color_buffer_refreshed_ = false;
}