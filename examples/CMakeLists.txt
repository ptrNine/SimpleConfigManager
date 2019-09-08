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