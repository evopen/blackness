set(TEST_TARGET ${PROJECT_NAME}_test)

add_executable(${TEST_TARGET} "test.cpp"  "video_generator_test.cpp" "../src/video_generator.cpp")
target_include_directories(${TEST_TARGET} PRIVATE ${SOURCE_DIR})
target_precompile_headers(${TEST_TARGET} PRIVATE ${SOURCE_DIR}/pch.h)

find_package(Qt5 COMPONENTS Widgets REQUIRED)
target_link_libraries(${TEST_TARGET} PRIVATE Qt5::Widgets)

find_package(GTest CONFIG REQUIRED)
target_link_libraries(${TEST_TARGET} PRIVATE GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)

include(GoogleTest)
gtest_discover_tests(${TEST_TARGET})