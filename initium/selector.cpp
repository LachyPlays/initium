#include "selector.hpp"

#include <set>
#include <algorithm>
#include <bit>

// Returns a vector of all queue families
std::vector<VkQueueFamilyProperties> getQueueFamilies(VkPhysicalDevice device) {
	uint32_t queue_family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);
	std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families.data());

	return queue_families;
}

// Maps a set of queue requests into their queueFamily/request pair, ignoring unsatisfiable requests
std::unordered_map<int, initium::QueueRequest> mapQueueFamilies(VkPhysicalDevice device, std::vector<initium::QueueRequest>& requests) {
	std::vector<VkQueueFamilyProperties> families = getQueueFamilies(device);

	// Holds the mapping between queue families and requests
	std::unordered_map<int, initium::QueueRequest> queue_map = {};

	// Loop over every request and create a mapping if possible
	// Try to choose the family with the smallest bit difference
	for (initium::QueueRequest& request : requests) {
		// A pair of the current best matching indice, and it's bit count
		int best_queue_indice = -1;
		int best_queue_bits = INT_MAX;

		// Find the best matching queue family, if any
		for (int i = 0; i < families.size(); i++) {
			// Check if all requested flags are in this queue, and that this queue is not empty
			bool required_flags = (families[i].queueFlags & request.flags) == request.flags;
			bool count_gt_z = families[i].queueCount > 0;

			// If present is not needed, this stays true
			bool present_capable = true;
			if (request.present_surface != VK_NULL_HANDLE) {
				// This is only run if present_surface was set
				VkBool32 present_supported = VK_FALSE;
				vkGetPhysicalDeviceSurfaceSupportKHR(device, i, request.present_surface, &present_supported);
				present_capable = present_supported;
			}


			if (required_flags && count_gt_z && present_capable) {
				std::bitset<32> bits(families[i].queueFlags);

				// Check if this has fewer bits/is more specific than the next best queue
				if (bits.count() < best_queue_bits) {
					// First, increment the queue count of the previously selected queue
					if (best_queue_indice >= 0) {
						families[best_queue_indice].queueFlags++;
					}

					// It does, so this is now our best queue
					best_queue_indice = i;
					best_queue_bits = bits.count();
					families[i].queueCount--;
				}
			}
		}

		// We have now scanned all queues, so add the best mapping if it exists
		if (best_queue_indice >= 0) {
			queue_map[best_queue_indice] = request;
		}
	}

	return queue_map;
}

bool areQueuesSupported(VkPhysicalDevice device, std::vector<initium::QueueRequest> requirements) {
	// If the number of mappings equals the number of requests, then all queues are supported
	return mapQueueFamilies(device, requirements).size() == requirements.size();
}

bool areFormatsSupported(VkPhysicalDevice device, std::vector<initium::ImageFormatRequirement> requirements) {
	// Can it satisfy all required image formats?
	for (initium::ImageFormatRequirement& format_requirement : requirements) {
		VkImageFormatProperties format_properties = {};

		VkResult result = vkGetPhysicalDeviceImageFormatProperties(device,
			format_requirement.format, format_requirement.type, format_requirement.tiling,
			format_requirement.usage_flags, format_requirement.create_flags, &format_properties);

		// Make sure that this format is supported
		switch (result) {
		case VK_SUCCESS:
			break;
		default:
			return false;
		}

		// Ensure all format properties match
		bool properties_match =
			((format_requirement.sample_flags & format_properties.sampleCounts) == format_requirement.sample_flags) &&
			(format_requirement.minimum_format_extent.width < format_properties.maxExtent.width) &&
			(format_requirement.minimum_format_extent.height < format_properties.maxExtent.height) &&
			(format_requirement.minimum_format_extent.depth < format_properties.maxExtent.depth) &&
			(format_requirement.minimum_array_layers < format_properties.maxArrayLayers) &&
			(format_requirement.minimum_mip_levels < format_properties.maxMipLevels) &&
			(format_requirement.minimum_resource_size < format_properties.maxResourceSize);
		if (!properties_match) { return false; }
	}

	// If everything is satisfied, return true
	return true;
}

bool areFeaturesSupported(VkPhysicalDevice device, VkPhysicalDeviceFeatures requirements) {
	// Get the devices supported features
	VkPhysicalDeviceFeatures features;
	vkGetPhysicalDeviceFeatures(device, &features);

	// Since features are just arrays of bools, loop through each feature and ensure that
	// every feature set to true in requirements is also true in the devies feature list
	for (int i = 0; i < sizeof(VkPhysicalDeviceFeatures) / 4; i++) {
		// I know, this is ridiculously unsafe
		if (((VkBool32*)&requirements)[i] == VK_TRUE) {
			if (((VkBool32*)&features)[i] == VK_FALSE) {
				return false;
			}
		}
	}

	// If we've looped through everything and still haven't found a mismatch, all features are supported
	return true;
}

bool areLimitsSatisfied(VkPhysicalDevice device, initium::LimitRequirements requirements) {
	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(device, &properties);

	VkPhysicalDeviceLimits& limits = properties.limits;

	// Return false if any limit is not satisfied
	if (limits.maxImageDimension1D < requirements.minimum_1d_texture_size) { return false; }
	if (limits.maxImageDimension2D < requirements.minimum_2d_texture_size) { return false; }
	if (limits.maxImageDimension3D < requirements.minimum_3d_texture_size) { return false; }

	// If no limit requirements are unmet, then return true
	return true;
}

// Check if a device is suitable based upon a series of requirements
initium::DeviceSuitability isDeviceSuitable(VkPhysicalDevice device, initium::DeviceRequirements requirements) {
	// Can it satisfy all queue requests?
	if (!areQueuesSupported(device, requirements.queue_requests)) { return initium::DeviceSuitability::UnsatisfiedQueues; }

	// Can it satisfy all format requests?
	if (!areFormatsSupported(device, requirements.required_formats)) { return initium::DeviceSuitability::UnsatisfiedFormats; }

	// Can it satisfy all required features?
	if (!areFeaturesSupported(device, requirements.required_features)) { return initium::DeviceSuitability::UnsatisfiedFeatures; }

	// Can it satisfy all required limits?
	if (!areLimitsSatisfied(device, requirements.limit_requirements)) { return initium::DeviceSuitability::UnsatisfiedLimits; }

	// If it can, our work is done
	return initium::DeviceSuitability::Suitable;
}

int getDeviceScore(VkPhysicalDevice device) {
	int score = 0;

	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(device, &properties);
	if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) score += 500;
	if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) score += 50;
	score += properties.limits.maxImageDimension2D / 100;
	score += properties.limits.maxImageDimension3D / 100;

	return score;
}

std::expected<VkPhysicalDevice, std::string> pickPhysicalDevice(VkInstance instance, DeviceRequirements requirements) {
	uint32_t physical_device_count = 0;
	vkEnumeratePhysicalDevices(instance, &physical_device_count, nullptr);
	std::vector<VkPhysicalDevice> physical_devices(physical_device_count);
	vkEnumeratePhysicalDevices(instance, &physical_device_count, physical_devices.data());

	// Filter out devices which do not match our requirements
	std::vector<VkPhysicalDevice> candidates;
	for (VkPhysicalDevice device : physical_devices) {
		DeviceSuitability suitability = isDeviceSuitable(device, requirements);
		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(device, &properties);

		if (suitability == DeviceSuitability::Suitable) {
			candidates.push_back(device);
		}
		else {
			switch (suitability)
			{
			case initium::UnsatisfiedQueues:
				printf("[%s] Does not satisfy queue requirements\n", properties.deviceName);
				break;
			case initium::UnsatisfiedFormats:
				printf("[%s] Does not satisfy format requirements\n", properties.deviceName);
				break;
			case initium::UnsatisfiedFeatures:
				printf("[%s] Does not satisfy feature requirements\n", properties.deviceName);
				break;
			case initium::UnsatisfiedLimits:
				printf("[%s] Does not satify limit requirements\n", properties.deviceName);
				break;
			default:
				break;
			}
		}
	}
	if (candidates.empty()) return std::unexpected<std::string>("No valid devices found");

	// Score candidates
	std::map<int, VkPhysicalDevice> scored_candidates;
	for (VkPhysicalDevice device : candidates) {
		scored_candidates[getDeviceScore(device)] = device;
	}


	// Return the highest scored candidate
	return scored_candidates.rbegin()->second;
}