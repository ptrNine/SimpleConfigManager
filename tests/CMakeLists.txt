find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
    message(STATUS "Submodule update")
    execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            RESULT_VARIABLE GIT_SUBMOD_RESULT)
    if(NOT GIT_SUBMOD_RESULT EQUAL "0")
        message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
    endif()
else()
    message(FATAL_ERROR "Can't find git")
endif()

project(googletest-download NONE)

execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
        -DCMAKE_BUILD_TYPE=Release
        -DCMAKE_INSTALL_PREFIX=${PROJECT_BINARY_DIR}/../googletest
        -B${PROJECT_BINARY_DIR}/../googletest
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/../googletest
)
if(result)
    message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif()

execute_process(COMMAND ${CMAKE_COMMAND} --build . --target install
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/../googletest
)
if(result)
    message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif()


find_package(GTest PATHS "${PROJECT_BINARY_DIR}/../googletest/")

enable_testing()

include_directories(
        ${PROJECT_BINARY_DIR}/..
        ${PROJECT_SOURCE_DIR}/..
)

link_directories(${PROJECT_BINARY_DIR})

add_executable(Tests tests.cpp)
target_link_libraries(Tests Threads::Threads scmfs_static GTest::gtest GTest::gtest_main)

add_test(NAME Tests COMMAND Tests)

configure_file(
        ${PROJECT_SOURCE_DIR}/test.cfg
        ${PROJECT_BINARY_DIR}/test.cfg COPYONLY
)
