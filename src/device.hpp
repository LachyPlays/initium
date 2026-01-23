#ifndef DEVICE_HPP
#define DEVICE_HPP

#include <vulkan/vulkan.h>
#include <expected>
#include <string>
#include <tuple>
#include <vector>
#include <memory>
#include <optional>
#include <bitset>
#include <map>
#include <unordered_map>

namespace initium {
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

	// A request for a certain queue type
	struct QueueRequest {
		std::optional<VkQueue> queue = std::nullopt;
		uint32_t family_indice;
		VkQueueFlags flags = NULL;
		VkDeviceQueueCreateFlags create_flags = NULL;
		float priority = 1.0f;
	};

	struct SwapChainRequest {
		std::optional<VkSwapchainKHR> swap_chain = std::nullopt;
		std::vector<QueueRequest>* supported_queues;
		VkSurfaceKHR surface;
		VkExtent2D window_size;
		VkSurfaceFormatKHR format;
		VkPresentModeKHR present_mode;
		uint32_t surface_count = 2;
		VkSurfaceTransformFlagBitsKHR transform_flags = {};
		VkCompositeAlphaFlagBitsKHR alpha_flags = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		VkBool32 clipped = VK_TRUE;
		VkImageUsageFlags usage_flags = {};
		VkSharingMode sharing_mode = VK_SHARING_MODE_CONCURRENT;
	};

	// Requirements for a physical device
	struct DeviceRequirements {
		std::vector<QueueRequest>* queue_requests;
		std::vector<ImageFormatRequirement> formats = {};
		VkPhysicalDeviceFeatures features = {};
		LimitRequirements limits = {};
		std::vector<const char*> extensions = {};
	};

	struct DeviceParams {
		VkPhysicalDevice physical_device;
		VkPhysicalDeviceFeatures features = {};
		std::vector<const char*> extensions = {};
		std::vector<QueueRequest>* queue_requests = nullptr;
		bool enable_validation_layers = false;
	};

	struct DeviceResult {
		VkDevice device;
	};

	/*
	Selects a physical device based upon a set of requirements. Devices without the base requirements will not be seen as candidates in
	the selection process. Optional requirements will then be used to rank devices that pass the base requirements, before a final score is
	associated with any devices that pass all requirements equally well.

	!TODO! Optional requirements are not yet implemented. This part of the library is still in flux, thus this feature will be added in later.
	*/
	std::optional<VkPhysicalDevice> pick_physical_device(VkInstance instance, DeviceRequirements requirements);

	std::optional<VkDevice> createLogicalDevice(
		VkPhysicalDevice physical_device,
		VkPhysicalDeviceFeatures features,
		std::vector<const char*> extensions,
		std::vector<QueueRequest>& queue_requests,
		std::vector<SwapChainRequest>& swapchain_requests,
		bool enable_validation_layers
	);
}

#endif