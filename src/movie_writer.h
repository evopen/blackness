// Adapted from
// https://stackoverflow.com/questions/34511312/how-to-encode-a-video-from-several-images-generated-in-a-c-program-without-wri

#pragma once

#include <cairo/cairo.h>
#include <stdint.h>

#include <string>
#include <vector>

extern "C" {
//#include <x264.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/mathematics.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
}

class MovieWriter
{
    const unsigned int kWidth, kHeight;
    unsigned int iframe_;

    SwsContext* swsCtx_;
    AVOutputFormat* fmt_;
    AVStream* stream_;
    AVFormatContext* fc_;
    AVCodecContext* c_;
    AVPacket pkt_;

    AVFrame *rgbpic_, *yuvpic_;

    std::vector<uint8_t> pixels_;

    cairo_surface_t* cairo_surface_;

    AVRational av_rational_{1, 25};

public:
    MovieWriter(const std::string& filename, unsigned int kWidth, unsigned int kHeight);

    void AddFrame(const std::string& filename);

    void AddFrame(const uint8_t* pixels);

    ~MovieWriter();
};
