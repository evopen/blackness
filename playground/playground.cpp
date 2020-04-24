#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_write.h>

#include <exception>
#include <filesystem>
#include <iostream>
#include <thread>

namespace fs = std::filesystem;

class A
{
    void Print(int x) { std::cout << "sdf\n"; }

public:
    void Generate() {
        std::vector<std::thread> threads;
        for (int i = 0; i < 100; ++i)
        {
            threads.emplace_back(std::thread(&A::Print, this, i));
        }

        for (int i = 0; i < 100; ++i)
        {
            threads[i].join();
        }
    }
};

int main()
{
    try
    {
        A a;
        a.Generate();

    }
    catch (std::exception& e)
    {
        std::cerr << e.what();
    }
    return 0;
}