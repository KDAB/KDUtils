add_feature_info(KDCore-Test ON "KDCore Unit Tests")

include (${doctest_SOURCE_DIR}/scripts/cmake/doctest.cmake)

add_subdirectory(bytearray)
add_subdirectory(file)
add_subdirectory(dir)
add_subdirectory(elapsedtimer)
add_subdirectory(flags)
add_subdirectory(url)

if (KDCORE_CODE_COVERAGE)
    include(${PROJECT_SOURCE_DIR}/cmake/CodeCoverage.cmake)
    setup_target_for_coverage_gcovr_html(
    NAME KDCore_coverage
    EXECUTABLE ctest -L "KDCore"
    EXCLUDE
    "${CMAKE_BINARY_DIR}/_deps/*"
    "${CMAKE_BINARY_DIR}/src/*"
    )
endif()
