find_package(Qt5 COMPONENTS Widgets REQUIRED)
qt5_wrap_ui(SOURCES "gui/main_window.ui")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)


add_executable (blackness 
    "blackness.cpp" 
    "movie_writer.cpp" 
    "metric/schwarzschild.cpp" 
    "metric/flat.cpp" 
    "metric/common.cpp"
    "image_generator.cpp" 
    "gui/main_window.cpp"
    "gui/main_window.ui"
)

if(WIN32)
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
      set_property(TARGET blackness PROPERTY WIN32_EXECUTABLE true)
    endif()
endif(WIN32)


target_precompile_headers(blackness PRIVATE pch.h)
set_target_properties(blackness PROPERTIES UNITY_BUILD ON)
target_include_directories(blackness PRIVATE ${CMAKE_CURRENT_LIST_DIR})

find_package(cxxopts CONFIG REQUIRED)
target_link_libraries(blackness PRIVATE cxxopts::cxxopts)

find_package(glm CONFIG REQUIRED)
target_link_libraries(blackness PRIVATE glm)

find_package(unofficial-cairo CONFIG REQUIRED)
target_link_libraries(blackness PRIVATE unofficial::cairo::cairo unofficial::cairo::cairo-gobject)

find_package(GSL REQUIRED)
target_link_libraries(blackness PRIVATE GSL::gsl GSL::gslcblas)


target_link_libraries(blackness PRIVATE Qt5::Widgets)

find_package(OpenCV CONFIG REQUIRED)
target_link_libraries(blackness PRIVATE ${OpenCV_LIBS})

find_library(AVDEVICE_LIBRARY avdevice)
find_library(AVFILTER_LIBRARY avfilter)
find_library(SWSCALE_LIBRARY swscale)
find_library(AVFORMAT_LIBRARY avformat)
find_library(AVCODEC_LIBRARY avcodec)
find_library(SWRESAMPLE_LIBRARY swresample)
find_library(AVRESAMPLE_LIBRARY avresample)
find_library(AVUTIL_LIBRARY avutil)
target_link_libraries(blackness PRIVATE
    ${AVDEVICE_LIBRARY} 
    ${AVFILTER_LIBRARY}
    ${SWSCALE_LIBRARY} 
    ${AVFORMAT_LIBRARY} 
    ${AVCODEC_LIBRARY} 
    ${SWRESAMPLE_LIBRARY}
    ${AVRESAMPLE_LIBRARY} 
    ${AVUTIL_LIBRARY}
)