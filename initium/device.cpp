#include "device.hpp"

#include <set>

// Returns a set including each type of family included
std::set<VkQueueFlags> getQueueFamilies(VkPhysicalDevice device) {
	uint32_t queue_family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);
	std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families.data());

	std::set<VkQueueFlags> unique_queue_families;
	for (const VkQueueFamilyProperties& queue_family : queue_families) {
		unique_queue_families.insert(queue_family.queueFlags);
	}

	return unique_queue_families;
}


// Check if a device is suitable based upon a series of requirements
bool isDeviceSuitable(VkPhysicalDevice device, initium::DeviceRequirements requirements) {
	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(device, &properties);

	if (requirements.require_dedicated_gpu && !properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) return false;

	// Queue family check
	std::set<VkQueueFlags> queue_families = getQueueFamilies(device);
	VkQueueFlags available_queue_types = 0;
	for (VkQueueFlags requested_queue : queue_families) {
		available_queue_types |= requested_queue;
	}
	if ((requirements.required_queues & available_queue_types) != requirements.required_queues) return false;
	if (requirements.require_dedicated_transfer && !queue_families.contains(VK_QUEUE_TRANSFER_BIT)) return false;

	// If no requirements are left unsatisfied, our work is done
	return true;
}

int getDeviceScore(VkPhysicalDevice device) {
	int score = 0;

	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(device, &properties);
	if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) score += 500;
	if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) score += 50;
	score += properties.limits.maxImageDimension2D / 100;
	score += properties.limits.maxImageDimension3D / 100;

	std::set<VkQueueFlags> queue_families = getQueueFamilies(device);
	std::bitset<32> available_queue_types;
	for (VkQueueFlags requested_queue : queue_families) {
		available_queue_types |= requested_queue;
	}
	score += available_queue_types.count() * 10;

	return score;
}

namespace initium {
	std::optional<VkPhysicalDevice> pickPhysicalDevice(VkInstance instance, DeviceRequirements requirements) {
		uint32_t physical_device_count = 0;
		vkEnumeratePhysicalDevices(instance, &physical_device_count, nullptr);
		std::vector<VkPhysicalDevice> physical_devices(physical_device_count);
		vkEnumeratePhysicalDevices(instance, &physical_device_count, physical_devices.data());

		// Filter out devices which do not match our requirements
		std::vector<VkPhysicalDevice> candidates;
		for (VkPhysicalDevice device : physical_devices) {
			if (isDeviceSuitable(device, requirements)) {
				candidates.push_back(device);
			}
		}
		if (candidates.empty()) return std::nullopt;

		// Score candidates
		std::map<int, VkPhysicalDevice> scored_candidates;
		for (VkPhysicalDevice device : candidates) {
			scored_candidates[getDeviceScore(device)] = device;
		}

		return scored_candidates.rbegin()->second;
	}

	Device::Device(VkInstance instance, VkPhysicalDevice physical_device, VkDevice device) : instance_(instance), physical_device_(physical_device), device_(device) {}

	Device::~Device() {
		if (device_ != VK_NULL_HANDLE) {
			vkDestroyDevice(device_, nullptr);
		}
	}
}
