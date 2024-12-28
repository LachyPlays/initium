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
#include "selector.hpp"

namespace initium {
	std::optional<VkDevice> createLogicalDevice(
		VkPhysicalDevice physical_device,
		VkPhysicalDeviceFeatures features,
		std::vector<const char*> extensions,
		std::vector<QueueRequest>& queue_requests,
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