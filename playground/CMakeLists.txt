add_executable(playground "playground.cpp")

find_package(glm CONFIG REQUIRED)
target_link_libraries(playground PRIVATE glm)