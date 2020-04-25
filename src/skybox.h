#pragma once

#include "pch.h"


struct Skybox
{
    std::shared_ptr<cv::Mat> front;
    std::shared_ptr<cv::Mat> back;
    std::shared_ptr<cv::Mat> top;
    std::shared_ptr<cv::Mat> bottom;
    std::shared_ptr<cv::Mat> left;
    std::shared_ptr<cv::Mat> right;
};