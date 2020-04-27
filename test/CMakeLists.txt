set(TEST_TARGET ${PROJECT_NAME}_test)

set(SOURCE_FILES 
    "../src/image_generator.cpp"
    "../src/movie_writer.cpp"
    "../src/video_generator.cpp"
    "../src/metric/common.cpp"
    "../src/metric/flat.cpp"
    "../src/metric/schwarzschild.cpp"
)

add_executable(${TEST_TARGET} "test.cpp"  "video_generator_test.cpp" 
    ${SOURCE_FILES})
target_include_directories(${TEST_TARGET} PRIVATE ${SOURCE_DIR})
target_precompile_headers(${TEST_TARGET} PRIVATE ${SOURCE_DIR}/pch.h)

find_package(Qt5 COMPONENTS Widgets REQUIRED)
target_link_libraries(${TEST_TARGET} PRIVATE Qt5::Widgets)

find_package(GTest CONFIG REQUIRED)
target_link_libraries(${TEST_TARGET} PRIVATE GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)

find_package(unofficial-cairo CONFIG REQUIRED)
target_link_libraries(${TEST_TARGET} PRIVATE unofficial::cairo::cairo unofficial::cairo::cairo-gobject)

find_package(OpenCV CONFIG REQUIRED)
target_link_libraries(${TEST_TARGET} PRIVATE ${OpenCV_LIBS})

find_package(GSL REQUIRED)
target_link_libraries(${TEST_TARGET} PRIVATE GSL::gsl GSL::gslcblas)


find_package(OpenCV CONFIG REQUIRED)
target_link_libraries(${TEST_TARGET} PRIVATE ${OpenCV_LIBS})

#find_library(AVDEVICE_LIBRARY avdevice)
#find_library(AVFILTER_LIBRARY avfilter)
find_library(SWSCALE_LIBRARY swscale)
find_library(AVFORMAT_LIBRARY avformat)
find_library(AVCODEC_LIBRARY avcodec)
#find_library(SWRESAMPLE_LIBRARY swresample)
#find_library(AVRESAMPLE_LIBRARY avresample)
find_library(AVUTIL_LIBRARY avutil)
target_link_libraries(${TEST_TARGET} PRIVATE
#    ${AVDEVICE_LIBRARY} 
#    ${AVFILTER_LIBRARY}
    ${SWSCALE_LIBRARY} 
    ${AVFORMAT_LIBRARY} 
    ${AVCODEC_LIBRARY} 
#    ${SWRESAMPLE_LIBRARY}
#    ${AVRESAMPLE_LIBRARY} 
    ${AVUTIL_LIBRARY}
)

include(GoogleTest)
gtest_discover_tests(${TEST_TARGET})
