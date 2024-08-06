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

#include "queue.hpp"

namespace initium {
	// A request for a certain queue type
	struct QueueRequest {
		std::unique_ptr<Queue> *queue = nullptr;
		VkQueueFlags flags = NULL;
		VkSurfaceKHR present_surface = VK_NULL_HANDLE;
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
		std::vector<QueueRequest> queue_requests = {};
		std::vector<ImageFormatRequirement> required_formats = {};
		VkPhysicalDeviceFeatures required_features = {};
		LimitRequirements limit_requirements = {};
		std::vector<const char*> required_extensions = {};
		bool enable_validation_layers = false;
	};

	enum DeviceSuitability {
		Suitable,
		UnsatisfiedQueues,
		UnsatisfiedFormats,
		UnsatisfiedFeatures,
		UnsatisfiedLimits,
	};

	std::expected<VkPhysicalDevice, std::string> pickPhysicalDevice(VkInstance instance, DeviceRequirements requirements);
	std::optional<VkDevice> createLogicalDevice(
		VkPhysicalDevice physical_device,
		VkPhysicalDeviceFeatures features,
		std::vector<const char*> extensions,
		std::vector<QueueRequest> queue_requests,
		bool enable_validation_layers
	);

	class Device {
	public:
		Device(VkInstance instance, VkPhysicalDevice physical_device, VkDevice device);
		~Device();
	private:
		VkInstance instance_ = VK_NULL_HANDLE;
		VkPhysicalDevice physical_device_ = VK_NULL_HANDLE;
		VkDevice device_ = VK_NULL_HANDLE;
	};
}

#endif