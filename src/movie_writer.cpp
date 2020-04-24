// Adapted from
// https://stackoverflow.com/questions/34511312/how-to-encode-a-video-from-several-images-generated-in-a-c-program-without-wri

#include "movie_writer.h"

#include <vector>

using namespace std;

// One-time initialization.
class FFmpegInitialize
{
public:
    FFmpegInitialize()
    {
        // Loads the whole database of available codecs and formats.
        av_register_all();
    }
};

static FFmpegInitialize ffmpeg_initialize;

MovieWriter::MovieWriter(const string& filename_, const unsigned int kWidth, const unsigned int kHeight)
    :

      kWidth(kWidth), kHeight(kHeight), iframe_(0), pixels_(4 * kWidth * kHeight)

{
    cairo_surface_ = cairo_image_surface_create_for_data((unsigned char*) &pixels_[0], CAIRO_FORMAT_RGB24, kWidth,
        kHeight, cairo_format_stride_for_width(CAIRO_FORMAT_RGB24, kWidth));

    // Preparing to convert my generated RGB images to YUV frames.
    swsCtx_ = sws_getContext(
        kWidth, kHeight, AV_PIX_FMT_RGB24, kWidth, kHeight, AV_PIX_FMT_YUV420P, SWS_FAST_BILINEAR, NULL, NULL, NULL);

    // Preparing the data concerning the format and codec,
    // in order to write properly the header, frame data and end of file.
    const char* fmtext     = "mp4";
    const string kFilename = filename_ + "." + fmtext;
    fmt_                   = av_guess_format(fmtext, NULL, NULL);
    avformat_alloc_output_context2(&fc_, NULL, NULL, kFilename.c_str());

    // Setting up the codec.
    AVCodec* codec    = avcodec_find_encoder_by_name("libvpx-vp9");
    AVDictionary* opt = NULL;
    av_dict_set(&opt, "preset", "slow", 0);
    av_dict_set(&opt, "crf", "20", 0);
    stream_          = avformat_new_stream(fc_, codec);
    c_               = stream_->codec;
    c_->width        = kWidth;
    c_->height       = kHeight;
    c_->pix_fmt      = AV_PIX_FMT_YUV420P;
    c_->time_base    = av_rational_;
    c_->thread_count = 12;

    // Setting up the format, its stream(s),
    // linking with the codec(s) and write the header.
    if ((fc_->oformat->flags & AVFMT_GLOBALHEADER) != 0)
    {
        // Some formats require a global header.
        c_->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }
    avcodec_open2(c_, codec, &opt);
    av_dict_free(&opt);

    // Once the codec is set up, we need to let the container know
    // which codec are the streams using, in this case the only (video) stream.
    stream_->time_base = av_rational_;
    av_dump_format(fc_, 0, kFilename.c_str(), 1);
    avio_open(&fc_->pb, kFilename.c_str(), AVIO_FLAG_WRITE);
    int ret = avformat_write_header(fc_, &opt);
    av_dict_free(&opt);

    // Preparing the containers of the frame data:
    // Allocating memory for each RGB frame, which will be lately converted to YUV.
    rgbpic_         = av_frame_alloc();
    rgbpic_->format = AV_PIX_FMT_RGB24;
    rgbpic_->width  = kWidth;
    rgbpic_->height = kHeight;
    ret             = av_frame_get_buffer(rgbpic_, 1);

    // Allocating memory for each conversion output YUV frame.
    yuvpic_         = av_frame_alloc();
    yuvpic_->format = AV_PIX_FMT_YUV420P;
    yuvpic_->width  = kWidth;
    yuvpic_->height = kHeight;
    ret             = av_frame_get_buffer(yuvpic_, 1);

    // After the format, code and general frame data is set,
    // we can write the video in the frame generation loop:
    // std::vector<uint8_t> B(width*height*3);
}

void MovieWriter::AddFrame(const string& filename)
{
    const string::size_type kP(filename.find_last_of('.'));
    string ext;
    if (kP != -1)
    {
        ext = filename.substr(kP);
    }

    if (ext == ".png")
    {
        cairo_surface_t* img = cairo_image_surface_create_from_png(filename.c_str());

        int imgw = cairo_image_surface_get_width(img);
        int imgh = cairo_image_surface_get_height(img);

        cairo_t* cr = cairo_create(cairo_surface_);
        cairo_scale(cr, (float) kWidth / imgw, (float) kHeight / imgh);
        cairo_set_source_surface(cr, img, 0, 0);
        cairo_paint(cr);
        cairo_destroy(cr);
        cairo_surface_destroy(img);

        unsigned char* data = cairo_image_surface_get_data(cairo_surface_);

        memcpy(&pixels_[0], data, pixels_.size());
    }
    else
    {
        fprintf(stderr, "The \"%s\" format is not supported\n", ext.c_str());
        exit(-1);
    }

    AddFrame((uint8_t*) &pixels_[0]);
}

void MovieWriter::AddFrame(const uint8_t* pixels)
{
    int channels = 3;
    // The AVFrame data will be stored as RGBRGBRGB... row-wise,
    // from left to right and from top to bottom.
    for (unsigned int y = 0; y < kHeight; y++)
    {
        for (unsigned int x = 0; x < kWidth; x++)
        {
            // rgbpic->linesize[0] is equal to width.
            rgbpic_->data[0][y * rgbpic_->linesize[0] + 3 * x + 0] = pixels[y * channels * kWidth + channels * x + 0];
            rgbpic_->data[0][y * rgbpic_->linesize[0] + 3 * x + 1] = pixels[y * channels * kWidth + channels * x + 1];
            rgbpic_->data[0][y * rgbpic_->linesize[0] + 3 * x + 2] = pixels[y * channels * kWidth + channels * x + 2];
        }
    }

    // Not actually scaling anything, but just converting
    // the RGB data to YUV and store it in yuvpic.
    sws_scale(swsCtx_, rgbpic_->data, rgbpic_->linesize, 0, kHeight, yuvpic_->data, yuvpic_->linesize);

    av_init_packet(&pkt_);
    pkt_.data = NULL;
    pkt_.size = 0;

    // The PTS of the frame are just in a reference unit,
    // unrelated to the format we are using. We set them,
    // for instance, as the corresponding frame number.
    yuvpic_->pts = iframe_;

    int got_output;
    int ret = avcodec_encode_video2(c_, &pkt_, yuvpic_, &got_output);
    if (got_output != 0)
    {
        fflush(stdout);

        // We set the packet PTS and DTS taking in the account our FPS (second argument),
        // and the time base that our selected format uses (third argument).
        av_packet_rescale_ts(&pkt_, av_rational_, stream_->time_base);

        pkt_.stream_index = stream_->index;
        printf("Writing frame %d (size = %d)\n", iframe_++, pkt_.size);

        // Write the encoded frame to the mp4 file.
        av_interleaved_write_frame(fc_, &pkt_);
        av_packet_unref(&pkt_);
    }
}

MovieWriter::~MovieWriter()
{
    // Writing the delayed frames:
    for (int got_output = 1; got_output != 0;)
    {
        int ret = avcodec_encode_video2(c_, &pkt_, NULL, &got_output);
        if (got_output != 0)
        {
            fflush(stdout);
            av_packet_rescale_ts(&pkt_, av_rational_, stream_->time_base);
            pkt_.stream_index = stream_->index;
            printf("Writing frame %d (size = %d)\n", iframe_++, pkt_.size);
            av_interleaved_write_frame(fc_, &pkt_);
            av_packet_unref(&pkt_);
        }
    }

    // Writing the end of the file.
    av_write_trailer(fc_);

    // Closing the file.
    if ((fmt_->flags & AVFMT_NOFILE) == 0)
    {
        avio_closep(&fc_->pb);
    }
    avcodec_close(stream_->codec);

    // Freeing all the allocated memory:
    sws_freeContext(swsCtx_);
    av_frame_free(&rgbpic_);
    av_frame_free(&yuvpic_);
    avformat_free_context(fc_);

    cairo_surface_destroy(cairo_surface_);
}
