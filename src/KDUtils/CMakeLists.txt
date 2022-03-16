set(SOURCES
    bytearray.cpp
    dir.cpp
    elapsedtimer.cpp
    file.cpp
    url.cpp
    )

set(HEADERS
    bytearray.h
    dir.h
    elapsedtimer.h
    file.h
    flags.h
    logger.h
    url.h
    )

add_library(KDUtils ${SOURCES} ${HEADERS})
add_library(KDUtils::KDUtils ALIAS KDUtils)
target_link_libraries(KDUtils PUBLIC spdlog::spdlog)

target_include_directories(KDUtils PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../>
    $<INSTALL_INTERFACE:include/>
    )

target_include_directories(KDUtils PRIVATE
    ${whereami_SOURCE_DIR}/src
    )
target_sources(KDUtils PRIVATE ${whereami_SOURCE_DIR}/src/whereami.c)


set_target_properties(KDUtils
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN 1
)

include(GenerateExportHeader)
generate_export_header(KDUtils BASE_NAME kdutils)
configure_file(${CMAKE_CURRENT_BINARY_DIR}/kdutils_export.h ${CMAKE_BINARY_DIR}/include/KDUtils/kdutils_export.h)
target_include_directories(KDUtils PUBLIC $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdutils_export.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/KDUtils)

set(project_config_in "${CMAKE_CURRENT_LIST_DIR}/cmake/KDUtilsConfig.cmake.in")
set(project_config_out "${CMAKE_CURRENT_BINARY_DIR}/KDUtilsConfig.cmake")

install (
    TARGETS KDUtils
    EXPORT KDUtils
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

install (EXPORT KDUtils DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/KDUtils NAMESPACE KDUtils:: FILE KDUtilsConfigTargets.cmake)
include(CMakePackageConfigHelpers)
configure_file("${project_config_in}" "${project_config_out}" @ONLY)
install(FILES "${project_config_out}" DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/KDUtils)
install(FILES ${HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/KDUtils)

if (KDUTILS_CODE_COVERAGE)
    include(${PROJECT_SOURCE_DIR}/cmake/CodeCoverage.cmake)
    append_coverage_compiler_flags()
endif()
