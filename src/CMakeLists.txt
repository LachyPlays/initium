add_library(initium STATIC)

target_sources(initium PRIVATE 
    instance.cpp
    framebuffer.cpp
    device.cpp
    pipeline.cpp
    renderpass.cpp
    command.cpp
    buffer.cpp
    image.cpp
    descriptor.cpp
    allocation.cpp
)

target_include_directories(initium
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(initium
    PUBLIC Vulkan::Vulkan
)

if (MSVC)
    target_compile_options(initium PRIVATE /W4)
endif()
