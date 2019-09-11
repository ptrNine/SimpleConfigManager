macro(scm_add_examples)
    foreach(name ${ARGN})
        message(${name})
        add_executable(${name} ${name}.cpp)
        target_link_libraries(${name} scmfs_static)

        install(FILES ${name}.cpp DESTINATION ${SCM_EXAMPLES_INSTALL_DIR}/src)
        install(TARGETS ${name} DESTINATION ${SCM_EXAMPLES_INSTALL_DIR})
    endforeach()
endmacro()

include_directories(${PROJECT_SOURCE_DIR})
link_directories(${PROJECT_BINARY_DIR})


file(GLOB _cfg_files "*.cfg")

foreach(_file ${_cfg_files})
    file(RELATIVE_PATH _bin_file ${CMAKE_CURRENT_SOURCE_DIR} ${_file})
    configure_file(${_file} ${PROJECT_BINARY_DIR}/examples/${_bin_file})
endforeach()

install(FILES ${_cfg_files} DESTINATION ${SCM_EXAMPLES_INSTALL_DIR})

scm_add_examples(
        hello_config
        custom_types
        custom_typedefs
        custom_namespace
        custom_assert
        example1
)

message(noinlne)
set(noinline_src
        noinline_example/test1.cpp
        noinline_example/test2.cpp
        noinline_example/noinline.cpp
        noinline_example/cfg.cpp
)
set(noinline_includes
        noinline_example/test1.hpp
        noinline_example/test2.hpp
        noinline_example/cfg.hpp
)
add_executable(noinline ${noinline_src})
target_link_libraries(noinline scmfs_static)
install(FILES ${noinline_src} ${noinline_includes} DESTINATION ${SCM_EXAMPLES_INSTALL_DIR}/src/noinline_example)
install(TARGETS noinline DESTINATION ${SCM_EXAMPLES_INSTALL_DIR})

