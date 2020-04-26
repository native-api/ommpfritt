function(generate_registers classes)
    SET(generated_src ${CMAKE_BINARY_DIR}/generated)
    SET(generator ${CMAKE_SOURCE_DIR}/build-scripts/generate-code.py)
    file(MAKE_DIRECTORY ${generated_src})
    include_directories(AFTER ${generated_src})

    add_custom_target(registers)
    foreach(R IN LISTS classes)
        SET(generated_cpp ${CMAKE_BINARY_DIR}/generated/register_${R}.cpp)
        SET(spec_file ${CMAKE_SOURCE_DIR}/lists/${R}.lst)
        add_custom_command(
            OUTPUT ${generated_cpp}
            COMMAND ${PYTHON_EXECUTABLE} ${generator} ${spec_file} ${generated_cpp}
            DEPENDS ${spec_file} ${generator}
            COMMENT "Generating ${R} register."
            VERBATIM
        )
        add_custom_target(${R}_register DEPENDS ${generated_cpp})
        add_dependencies(registers ${R}_register)
    endforeach()
endfunction()

