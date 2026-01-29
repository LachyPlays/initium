find_program(GLSLC glslc HINTS
    $ENV{VULKAN_SDK}/Bin
    $ENV{VULKAN_SDK}/Bin32
)

if (NOT GLSLC)
    message(FATAL_ERROR "glslc not found")
endif()


function(compile_glsl TARGET)
    set(SHADER_OUTPUT_DIR
        ${CMAKE_CURRENT_BINARY_DIR}/shaders
    )

    file(MAKE_DIRECTORY ${SHADER_OUTPUT_DIR})

    foreach(SHADER ${ARGN})
        get_filename_component(SHADER_NAME ${SHADER} NAME)
        set(SPIRV ${SHADER_OUTPUT_DIR}/${SHADER_NAME}.spv)

        add_custom_command(
            OUTPUT ${SPIRV}
            COMMAND ${GLSLC}
                    ${SHADER}
                    -g 
                    -o ${SPIRV}
            DEPENDS ${SHADER}
            COMMENT "Compiling shader ${SHADER_NAME}"
            VERBATIM
        )

        list(APPEND SPIRV_BINARIES ${SPIRV})
    endforeach()

    add_custom_target(
        ${TARGET}_shaders
        DEPENDS ${SPIRV_BINARIES}
    )

    add_dependencies(${TARGET} ${TARGET}_shaders)
endfunction()

add_subdirectory(glfw)
add_subdirectory(glm)

# Add all sample folders here
add_subdirectory(hello_triangle)
add_subdirectory(hello_cube)
add_subdirectory(textured_cube)