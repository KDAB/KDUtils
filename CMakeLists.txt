cmake_minimum_required(VERSION 3.12)
project(KDCore CXX C)

set(BUILD_SHARED_LIBS ON)

include(FeatureSummary)
include(CMakeDependentOption)
include(cmake/dependencies.cmake)
include(GNUInstallDirs)
include(CTest)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(src/KDCore)

option(KDCORE_BUILD_TESTS ON)
if (KDCORE_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
