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

namespace initium {
	struct DeviceRequirements {
		VkQueueFlags required_queues = {};
		bool require_dedicated_gpu = false;
		bool require_dedicated_transfer = false;
	};

	std::optional<VkPhysicalDevice> pickPhysicalDevice(VkInstance instance, DeviceRequirements requirements);


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