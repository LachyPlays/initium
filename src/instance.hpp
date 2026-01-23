#ifndef INSTANCE_HPP
#define INSTANCE_HPP

#include <vulkan/vulkan.h>
#include <expected>
#include <string>
#include <tuple>
#include <vector>
#include <memory>

#include "device.hpp"

VkResult createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

namespace initium {
	struct InstanceParams {
		const char* application_name = "Initium";
		std::tuple<int, int, int> application_version = { 0, 0, 0 };
		std::vector<const char*> extensions = {};
		bool enable_validation_layers = false;
	};

	struct InstanceResult {
		VkInstance instance;
		VkDebugUtilsMessengerEXT debug_messenger;
	};

	std::optional<InstanceResult> createInstance(InstanceParams params);
}
#endif