#ifndef SELECTOR_HPP
#define SELECTOR_HPP

#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_VULKAN
#include "glfw/include/GLFW/glfw3.h"

#include <expected>
#include <string>
#include <tuple>
#include <vector>
#include <memory>
#include <optional>
#include <bitset>
#include <map>
#include <unordered_map>

#include "queue.hpp"

namespace initium {
	// A request for a certain queue type
	struct QueueRequest {
		std::optional<Queue> queue = std::nullopt;
		VkQueueFlags flags = NULL;
		VkDeviceQueueCreateFlags create_flags = NULL;
		VkSurfaceKHR present_surface = VK_NULL_HANDLE;
		float priority = 1.0f;
	};

	// A requirement for a certain image format and associated properties
	struct ImageFormatRequirement {
		VkFormat format;
		VkImageType type;
		VkImageTiling tiling;
		VkImageUsageFlags usage_flags;
		VkImageCreateFlags create_flags = {};
		VkSampleCountFlags sample_flags = VK_SAMPLE_COUNT_1_BIT;
		VkDeviceSize minimum_resource_size = 0;
		VkExtent3D minimum_format_extent = { 0, 0, 0 };
		uint32_t minimum_mip_levels = 0;
		uint32_t minimum_array_layers = 0;
	};

	struct LimitRequirements {
		uint32_t minimum_1d_texture_size = 0;
		uint32_t minimum_2d_texture_size = 0;
		uint32_t minimum_3d_texture_size = 0;
	};

	// Requirements for a physical device
	struct DeviceRequirements {
		std::vector<QueueRequest>& queue_requests;
		std::vector<ImageFormatRequirement> formats = {};
		VkPhysicalDeviceFeatures features = {};
		LimitRequirements limits = {};
		std::vector<const char*> extensions = {};
	};

	std::expected<VkPhysicalDevice, std::string> pickPhysicalDevice(VkInstance instance, DeviceRequirements requirements, DeviceRequirements optional_requirements);
}

#endif
