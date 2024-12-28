#include "device.hpp"

#include <stdint.h>
#include <set>
#include <algorithm>
#include <bit>

namespace initium {
	// Returns an array of queue family indices, matching the array of queue requests. If the array size does not match, some requests could not be satisfied. 
	std::vector<int> createQueueMapping(VkPhysicalDevice device, std::vector<QueueRequest>& queue_requests) {
		// Enumarate all of the device's queue families
		uint32_t queue_family_count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);
		std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families.data());


		// Check each queue request and create a mapping for it, prioritising using different queue families for each request
		std::vector<int> queue_mapping;
		for (int request_index = 0; request_index < queue_requests.size(); request_index++) {
			auto request = queue_requests[request_index];
			// ! TODO ! Try to pick different families for each request.
			// Scan through queue families, finding the queue with the closest match (least flags)
			unsigned int lowest_bitcount = UINT_MAX;
			std::optional<int> queue_family_indice{};
			unsigned int queue_index = 0;
			for (auto& family : queue_families) {
				// Does this family have any queues left?
				if (family.queueCount > 0) {
					// Does this family support all required flags?
					if ((family.queueFlags & request.flags) == request.flags) {
						// Does this family support the requried present surface (if necessary)
						if (request.present_surface != VK_NULL_HANDLE) {
							VkBool32 surface_supported = VK_FALSE;
							vkGetPhysicalDeviceSurfaceSupportKHR(device, queue_index, request.present_surface, &surface_supported);
							if (!surface_supported) { queue_index++; continue; } // Skip this family if it isn't
						}
						// Is this the closest matching queue? (fewest set bits)
						auto flag_bits = std::bitset<sizeof(VkQueueFlags) * 8>(family.queueFlags);
						if (flag_bits.count() < lowest_bitcount) {
							// If it is, then set this as the newest, best matching queue family
							lowest_bitcount = flag_bits.count();
							queue_family_indice = queue_index;
						}
					}
				}
				queue_index++;
			}

			// If we found a valid family, create a mapping and decrement the number of available queues for a given family
			if (queue_family_indice.has_value()) {
				queue_mapping.push_back(queue_family_indice.value());
				queue_families[queue_family_indice.value()].queueCount -= 1;
			}
		}

		return queue_mapping;
	}

	std::optional<VkDevice> createLogicalDevice(
		VkPhysicalDevice physical_device,
		VkPhysicalDeviceFeatures features,
		std::vector<const char*> extensions,
		std::vector<QueueRequest>& queue_requests,
		bool enable_validation_layers
	) {
		// Create an optimal queue mapping
		std::vector<int> queue_mapping = createQueueMapping(physical_device, queue_requests);
		if (queue_mapping.size() != queue_requests.size()) { return std::nullopt; }

		// Fill all queue create info structure
		std::vector<VkDeviceQueueCreateInfo> queue_create_info(queue_requests.size());
 		for (int i = 0; i < queue_requests.size(); i++) {
			queue_create_info[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queue_create_info[i].flags = queue_requests[i].create_flags;
			queue_create_info[i].queueFamilyIndex = queue_mapping[i];
			queue_create_info[i].pQueuePriorities = &queue_requests[i].priority;
			queue_create_info[i].queueCount = 1;
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
		std::unordered_map<int, int> current_indexes{};
		for (int i = 0; i < queue_requests.size(); i++) {
			// Make sure we have a zeroed current index for this family, if we haven't seen it before
			if (!current_indexes.contains(queue_mapping[i])) {
				current_indexes[queue_mapping[i]] = 0;
			}

			VkQueue queue = VK_NULL_HANDLE;
			vkGetDeviceQueue(device, queue_mapping[i], current_indexes[queue_mapping[i]]++, &queue);
			if (queue == VK_NULL_HANDLE) {
				return std::nullopt;
			}

			queue_requests[i].queue = Queue(queue, queue_mapping[i]);
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
