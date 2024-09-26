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
	for (unsigned int i = 0; i < glfw_extension_count; i++) { extensions[i] = glfw_extensions_raw[i]; };
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

	// Surface creation
	VkSurfaceKHR surface = VK_NULL_HANDLE;
	if (glfwCreateWindowSurface(instance.get()->get_raw_instance(), window, nullptr, &surface) != VK_SUCCESS) {
		std::cerr << "Failed to create window surface" << std::endl;
		return 1;
	}

	std::unique_ptr<initium::Queue> graphics_queue = nullptr;
	std::unique_ptr<initium::Queue> transfer_queue = nullptr;

	auto device_result = instance.get()->create_device({
		.queue_requests = {
			{.queue = &graphics_queue, .flags = VK_QUEUE_GRAPHICS_BIT, .present_surface = surface},
			{.queue = &transfer_queue, .flags = VK_QUEUE_TRANSFER_BIT}
		},
		.required_extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME}
		});
	if (!device_result.has_value()) {
		std::cerr << device_result.error() << std::endl;
		return 1;
	}
	std::unique_ptr<initium::Device> device = std::move(device_result.value());



	// Render loop
	printf("@@@@ INITIALISATION COMPLETE @@@@\n");
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
	}
	// Cleanup
	vkDestroySurfaceKHR(instance.get()->get_raw_instance(), surface, nullptr);
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}