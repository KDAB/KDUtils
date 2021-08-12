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

add_library(KDCore ${SOURCES} ${HEADERS})
add_library(KDCore::KDCore ALIAS KDCore)
target_link_libraries(KDCore PUBLIC spdlog::spdlog)

target_include_directories(KDCore PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../>
    $<INSTALL_INTERFACE:include/>
    )

target_include_directories(KDCore PRIVATE
    ${whereami_SOURCE_DIR}/src
    )
target_sources(KDCore PRIVATE ${whereami_SOURCE_DIR}/src/whereami.c)


set_target_properties(KDCore
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN 1
)

include(GenerateExportHeader)
generate_export_header(KDCore BASE_NAME kdcore)
configure_file(${CMAKE_CURRENT_BINARY_DIR}/kdcore_export.h ${CMAKE_BINARY_DIR}/include/KDCore/kdcore_export.h)
target_include_directories(KDCore PUBLIC $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>)

set(project_config_in "${CMAKE_CURRENT_LIST_DIR}/cmake/KDCoreConfig.cmake.in")
set(project_config_out "${CMAKE_CURRENT_BINARY_DIR}/KDCoreConfig.cmake")

install (
    TARGETS KDCore
    EXPORT KDCore
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

install (EXPORT KDCore DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/KDCore NAMESPACE KDCore:: FILE KDCoreConfigTargets.cmake)
include(CMakePackageConfigHelpers)
configure_file("${project_config_in}" "${project_config_out}" @ONLY)
install(FILES "${project_config_out}" DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/KDCore)
install(FILES ${HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/KDCore)

if (KDCORE_CODE_COVERAGE)
    include(${PROJECT_SOURCE_DIR}/cmake/CodeCoverage.cmake)
    append_coverage_compiler_flags()
endif()
