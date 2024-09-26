#include "device.hpp"

#include <set>
#include <algorithm>
#include <bit>



namespace initium {
	std::optional<VkDevice> createLogicalDevice(
		VkPhysicalDevice physical_device,
		VkPhysicalDeviceFeatures features,
		std::vector<const char*> extensions,
		std::vector<QueueRequest> queue_requests,
		bool enable_validation_layers
	) {
		// Queue creation
		std::vector<VkDeviceQueueCreateInfo> queue_create_info{};
		std::unordered_map<int, QueueRequest> queue_map = mapQueueFamilies(physical_device, queue_requests);
		float queue_priority = 1.0f;
		for (std::pair<const int, QueueRequest>& queue_mapping : queue_map) {
			VkDeviceQueueCreateInfo create{
				.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				.queueFamilyIndex = static_cast<uint32_t>(queue_mapping.first),
				.queueCount = 1,
				.pQueuePriorities = &queue_priority
			};

			queue_create_info.push_back(create);
		}

		// Validation layers
		std::vector<const char*> layers = {};
		if (enable_validation_layers) {
			layers.push_back("VK_LAYER_KHRONOS_validation");
		}

		// Logical device create info
		VkDeviceCreateInfo create_info{
			.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
			.queueCreateInfoCount = static_cast<uint32_t>(queue_create_info.size()),
			.pQueueCreateInfos = queue_create_info.data(),
			.enabledLayerCount = static_cast<uint32_t>(layers.size()),
			.ppEnabledLayerNames = layers.data(),
			.enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
			.ppEnabledExtensionNames = extensions.data(),
			.pEnabledFeatures = &features,	
		};

		// Device creation
		VkDevice device = VK_NULL_HANDLE;
		if (vkCreateDevice(physical_device, &create_info, nullptr, &device) != VK_SUCCESS) {
			return std::nullopt;
		}

		// Queue population
		for (std::pair<const int, QueueRequest>& queue_mapping : queue_map) {
			VkQueue raw_queue = VK_NULL_HANDLE;
			vkGetDeviceQueue(device, queue_mapping.first, 0, &raw_queue);

			*queue_mapping.second.queue = std::make_unique<Queue>(raw_queue, queue_mapping.first);
		}

		return device;
	}

	Device::Device(VkInstance instance, VkPhysicalDevice physical_device, VkDevice device) : instance_(instance), physical_device_(physical_device), device_(device) {}

	Device::~Device() {
		if (device_ != VK_NULL_HANDLE) {
			vkDestroyDevice(device_, nullptr);
		}
	}
}
