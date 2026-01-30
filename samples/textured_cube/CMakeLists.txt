find_package(Vulkan REQUIRED)

add_executable(textured_cube
    main.cpp
)

target_link_libraries(textured_cube
    PRIVATE
        initium
        glfw
        glm
        Vulkan::Vulkan
)

file(COPY vulkan_logo.jpg DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

compile_glsl(textured_cube
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders/shader.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders/shader.frag
)

target_compile_definitions(textured_cube
    PRIVATE GLFW_INCLUDE_VULKAN
)