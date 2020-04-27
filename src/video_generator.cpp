#include "video_generator.h"

#include "movie_writer.h"

#include <fstream>

void VideoGenerator::SetOutputFilePath(std::filesystem::path path)
{
    output_path_ = path;
}


void VideoGenerator::GenerateAndSave()
{
    rendering_ = true;
    abort_     = false;
    MovieWriter movie_writer(output_path_.stem().string(), img_generator_->Width(), img_generator_->Height());

    for (int i = 0; i < frames_; ++i)
    {
        if (abort_)
            return;
        img_generator_->GetCamera()->SetPosition(positions_[i]);
        img_generator_->GetCamera()->SetFrontAndUp(fronts_[i], ups_[i]);

        img_generator_->Generate();

        if (bloom_)
        {
            img_generator_->Bloom();
            movie_writer.AddFrame(img_generator_->ResultBuffer()->data);
        }
        else
        {
            movie_writer.AddFrame(img_generator_->ColorBuffer()->data);
        }
    }
}

void VideoGenerator::LoadPositions(std::filesystem::path& filename)
{
    positions_.clear();
    fronts_.clear();
    ups_.clear();

    std::string str;
    std::ifstream fs(filename);
    int type        = 0;
    int frame_count = 0;
    while (fs >> str)
    {
        frame_count++;
        std::stringstream ss(str);
        glm::dvec3 coord;
        for (int i = 0; i < 3; ++i)
        {
            ss >> coord[i];
            if (ss.peek() == ',')
                ss.ignore();
        }

        switch (type)
        {
        case 0:
            positions_.push_back(coord);
            break;
        case 1:
            fronts_.push_back(coord);
            break;
        case 2:
            ups_.push_back(coord);
            break;
        default:
            throw std::runtime_error("unknown type");
            break;
        }
        type = (type + 1) % 3;
    }
    frames_ = frame_count / 3;
}
