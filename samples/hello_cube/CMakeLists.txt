find_package(Vulkan REQUIRED)

add_executable(hello_cube
    main.cpp
)

target_include_directories(hello_cube
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/glm
)

target_link_libraries(hello_cube
    PRIVATE
        initium
        glfw
        Vulkan::Vulkan
)

compile_glsl(hello_cube
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders/shader.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders/shader.frag
)

target_compile_definitions(hello_cube
    PRIVATE GLFW_INCLUDE_VULKAN
)