#include "pch.h"

#include "camera.h"
#include "gui/main_window.h"
#include "image.h"
#include "image_generator.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_write.h>

struct Arguments
{
    std::filesystem::path skybox_folder_path;
    std::filesystem::path disk_texture_path;
    std::filesystem::path output_path;
    uint32_t width;
    uint32_t height;
    uint32_t samples;
    uint32_t threads;
    double disk_inner;
    double disk_outer;
    glm::dvec3 camera_pos;
    glm::dvec3 camera_lookat;
    std::filesystem::path camera_pos_path;
    bool has_blackhole;
    bool has_disk;
    bool animate;
};

void Parse(int argc, char* argv[], Arguments& args)
{
    try
    {
        std::filesystem::path filename = std::filesystem::path(argv[0]).filename();
        cxxopts::Options options(filename.string(), " - A Schwarzschild Blackhole Generator");

        std::vector<double> camera_pos_vec;
        std::vector<double> camera_lookat_vec;

        // clang-format off
        options.add_options()
            ("blackhole", "Has blackhole or not", cxxopts::value<bool>(args.has_blackhole))
            ("o,output", "Path generated image", cxxopts::value<std::filesystem::path>(args.output_path), "FILE")
            ("threads", "How many threads to launch", 
                cxxopts::value<uint32_t>(args.threads)->default_value(std::to_string(std::thread::hardware_concurrency())), 
                "NUM")
            ("h, help", "Print help");

        options.add_options("Required")
            ("skybox", "Path to skybox folder", cxxopts::value<std::filesystem::path>(args.skybox_folder_path), "PATH")
            ("width", "width for output file", cxxopts::value<uint32_t>(args.width), "NUM")
            ("height", "height for output file", cxxopts::value<uint32_t>(args.height), "NUM")
            ("samples", "samples for pixel", cxxopts::value<uint32_t>(args.samples), "NUM");

        options.add_options("Disk")
            ("disk", "Path to disk texture file", cxxopts::value<std::filesystem::path>(args.disk_texture_path), "FILE")
            ("disk-inner", "Disk inner radius", cxxopts::value<double>(args.disk_inner), "NUM")
            ("disk-outer", "Disk outer radius", cxxopts::value<double>(args.disk_outer), "NUM");

        options.add_options("Image")
            ("pos", "Camera position", cxxopts::value<std::vector<double>>(camera_pos_vec), "VECOTR")
            ("lookat", "Camera lookat", cxxopts::value<std::vector<double>>(camera_lookat_vec), "VECTOR");

        options.add_options("Video")
            ("pos-file", "Path to camera position file", cxxopts::value<std::filesystem::path>(args.camera_pos_path), "FILE");
        // clang-format on

        auto result = options.parse(argc, argv);

        if (result.count("help") != 0)
        {
            std::cout << options.help({"", "Required", "Image", "Video", "Disk"}) << std::endl;
            exit(0);
        }

        if (result.count("skybox") != 1 || result.count("samples") != 1 || result.count("width") != 1
            || result.count("height") != 1)
        {
            std::cout << "missing required arguments\n"
                      << options.help({"", "Required", "Image", "Video", "Disk"}) << "\n";
            exit(0);
        }

        if (result.count("output") != 1)
        {
            std::cout << "please specify an output file\n";
            exit(0);
        }

        if (result.count("pos") == 1 && result.count("lookat") == 1 && result.count("pos-file") == 0)  // image mode
        {
            args.animate = false;
            if (camera_lookat_vec.size() != 3)
            {
                std::cout << "camera lookat vector must be 3-d"
                          << "\n";
                exit(0);
            }
            if (camera_pos_vec.size() != 3)
            {
                std::cout << "camera position vector must be 3-d"
                          << "\n";
                exit(0);
            }
            for (int i = 0; i < 3; ++i)
            {
                args.camera_pos[i]    = camera_pos_vec[i];
                args.camera_lookat[i] = camera_lookat_vec[i];
            }
        }
        else if (result.count("pos") == 0 && result.count("lookat") == 0 && result.count("pos-file") == 1)
        {  // video mode
            args.animate = true;
            if (!std::filesystem::exists(args.camera_pos_path)
                || std::filesystem::is_regular_file(args.camera_pos_path))
            {
                std::cout << "camera position file not exist\n";
                exit(0);
            }
        }
        else
        {
            std::cout << "can either work in image mode or video mode\n";
            exit(0);
        }


        if (!std::filesystem::exists(args.skybox_folder_path))
        {
            std::cout << "skybox texture folder does not exist"
                      << "\n";
            exit(0);
        }

        if (!std::filesystem::is_directory(args.skybox_folder_path))
        {
            std::cout << "skybox option must be a directory"
                      << "\n";
            exit(0);
        }

        // processing disk options

        if (result.count("disk-outer") == 1 && result.count("disk-inner") == 1 && result.count("disk") == 1)
        {
            args.has_disk = true;
            if (!std::filesystem::exists(args.disk_texture_path))
            {
                std::cout << "accretion texture does not exist"
                          << "\n";
                exit(0);
            }

            if (!std::filesystem::is_regular_file(args.disk_texture_path))
            {
                std::cout << "disk texture must be a single file"
                          << "\n";
                exit(0);
            }
        }
        else
        {
            args.has_disk = false;
        }
    }
    catch (const cxxopts::OptionException& e)
    {
        std::cout << "error parsing options: " << e.what() << std::endl;
        exit(1);
    }
}

int main(int argc, char* argv[])
{
    try
    {
        if (argc == 1)
        {
            QApplication app(argc, argv);
            QGuiApplication::setApplicationDisplayName(MainWindow::tr("Blackness"));
            MainWindow main_window;

            main_window.show();
            return app.exec();
        }
        Arguments args;
        Parse(argc, argv, args);


        Camera camera(args.camera_pos, args.camera_lookat);

        ImageGenerator img_generator(args.skybox_folder_path, camera, args.samples, args.width, args.height);
        img_generator.SetThreads(args.threads);

        Blackhole* blackhole;
        if (args.has_blackhole)
        {
            blackhole = new Blackhole(glm::dvec3(0, 0, 0), args.disk_inner, args.disk_outer, args.disk_texture_path);
            img_generator.SetBlackhole(*blackhole);
        }

        img_generator.Generate();
        img_generator.SaveImage(args.output_path);
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << "\n";
    }

    return 0;
}