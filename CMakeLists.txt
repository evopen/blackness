# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.16)

project ("blackness" CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_FLAGS_RELEASE "-Ofast")

include(CTest)

set(SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/src")
set(TEST_DIR "${CMAKE_CURRENT_LIST_DIR}/test")

add_subdirectory (${SOURCE_DIR})

if(BUILD_TESTING)
	add_subdirectory(${TEST_DIR})
endif()


add_subdirectory ("playground")

set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "2")
set(CPACK_PACKAGE_VERSION_PATCH "0")

# Packaging support
set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION ".")
include(InstallRequiredSystemLibraries)

set(CPACK_SOURCE_GENERATOR "TGZ")
set(CPACK_GENERATOR "ZIP")
set(CPACK_STRIP_FILES true)

include(CPack)


