cmake_minimum_required(VERSION 3.10)
project(SCM)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic")

set(ScmIncludes
        scm/scm.hpp
        scm/scm_aton.hpp
        scm/scm_details.hpp
        scm/scm_filesystem.hpp
        scm/scm_types.hpp
        scm/scm_utils.hpp
        scm/scm_details_inl.hpp
)

set(SCM_INCLUDE_PATH "${CMAKE_INSTALL_PREFIX}/include")
set(SCM_EXAMPLES_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/share/scm-examples")
set(SCM_LIB_PATH "${CMAKE_INSTALL_PREFIX}/lib")
set(SCM_CMAKE_PATH "${SCM_LIB_PATH}/cmake/SCM")
set(SCM_NAMESPACE scm)

option(SCM_BUILD_TESTS "Build test programs" OFF)
option(SCM_BUILD_EXAMPLES "Build example programs" OFF)

add_library(scmfs_object OBJECT scm/scm_filesystem.cpp)
set_property(TARGET scmfs_object PROPERTY POSITION_INDEPENDENT_CODE 1)

add_library(scmfs SHARED $<TARGET_OBJECTS:scmfs_object>)
add_library(scmfs_static STATIC $<TARGET_OBJECTS:scmfs_object>)

install(TARGETS scmfs scmfs_static
        DESTINATION ${SCM_LIB_PATH}
        EXPORT SCMExports
)
install(EXPORT SCMExports DESTINATION ${SCM_CMAKE_PATH})

install(
    FILES
        ${PROJECT_BINARY_DIR}/UseSCM.cmake
        ${PROJECT_BINARY_DIR}/SCMConfig.cmake
    DESTINATION
        ${SCM_CMAKE_PATH}
)
configure_file(
        ${PROJECT_SOURCE_DIR}/CMake/UseSCM.cmake.in
        ${PROJECT_BINARY_DIR}/UseSCM.cmake COPYONLY
)

configure_file(
        ${PROJECT_SOURCE_DIR}/CMake/SCMConfig.cmake.in
        ${PROJECT_BINARY_DIR}/SCMConfig.cmake @ONLY
)

if(SCM_BUILD_TESTS)
    message("Build tests")
    add_subdirectory(tests)
endif()

if(SCM_BUILD_EXAMPLES)
    message("Build examples")
    add_subdirectory(examples)
endif()