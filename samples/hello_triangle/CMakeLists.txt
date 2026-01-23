find_package(Vulkan REQUIRED)

add_executable(hello_triangle
    main.cpp
)

target_include_directories(hello_triangle
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/glm
)

target_link_libraries(hello_triangle
    PRIVATE
        initium
        glfw
        Vulkan::Vulkan
)

compile_glsl(hello_triangle
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders/shader.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders/shader.frag
)

target_compile_definitions(hello_triangle
    PRIVATE GLFW_INCLUDE_VULKAN
)