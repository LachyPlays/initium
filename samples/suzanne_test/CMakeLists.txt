find_package(Vulkan REQUIRED)

add_executable(suzanne_test
    main.cpp
)

target_include_directories(suzanne_test
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/glm
)

target_link_libraries(suzanne_test
    PRIVATE
        initium
        glfw
        glm
        tinygltf
        Vulkan::Vulkan
)

compile_glsl(suzanne_test
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders/shader.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders/shader.frag
)

file(COPY suzanne DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

target_compile_definitions(suzanne_test
    PRIVATE GLFW_INCLUDE_VULKAN
)