#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_VULKAN
#include "glfw/include/GLFW/glfw3.h"

#include "instance.hpp"
#include <vector>
#include <iostream>

int main() {
	// Initialize GLFW
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	GLFWwindow* window = glfwCreateWindow(600, 400, "Initium test", NULL, NULL);

	uint32_t glfw_extension_count = 0;
	const char** glfw_extensions_raw = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
	std::vector<const char*> extensions(glfw_extension_count);
	for (int i = 0; i < glfw_extension_count; i++) { extensions[i] = glfw_extensions_raw[i]; };
	extensions.push_back("VK_EXT_debug_utils");

	// Initialize renderer
	auto instance_result = initium::create_instance(
		{ .application_name = "Initium test",
			.application_version = {1, 0, 0},
			.extensions = extensions,
			.enable_validation_layers = true
		});
	if (!instance_result.has_value()) {
		std::cerr << instance_result.error() << std::endl;
		return 1;
	}
	std::unique_ptr<initium::Instance> instance = std::move(instance_result.value());

	// Render loop
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
	}

	// Cleanup
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}