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

	VkPhysicalDeviceLimits limits = properties.limits;
	printf("%i %i %i\n", requirements.minimum_1d_texture_size, requirements.minimum_2d_texture_size, requirements.minimum_3d_texture_size);
	// Return false if any limit is not satisfied
	if (limits.maxImageDimension1D < requirements.minimum_1d_texture_size) { return false; }
	if (limits.maxImageDimension2D < requirements.minimum_2d_texture_size) { return false; }
	if (limits.maxImageDimension3D < requirements.minimum_3d_texture_size) { return false; }

	// If no limit requirements are unmet, then return true
	return true;
}

// Score a device based on a number of factors scaled by arbitrary constants
int get_score(VkPhysicalDevice device) {
	int score = 0;

	// Device type scoring
	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(device, &properties);
	if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) { score += 500; }

	// Limit based scoring
	VkPhysicalDeviceLimits limits = properties.limits;
	score += limits.maxFramebufferHeight / 1024;
	score += limits.maxFramebufferWidth / 1024;
	score += limits.maxBoundDescriptorSets / 4;
	score += limits.maxComputeWorkGroupCount[0] / 128;
	score += limits.maxComputeWorkGroupCount[1] / 128;
	score += limits.maxComputeWorkGroupCount[2] / 128;
	score += limits.maxComputeSharedMemorySize / 4096;

	// Feature based scoring
	VkPhysicalDeviceFeatures features{};
	vkGetPhysicalDeviceFeatures(device, &features);
	for (int i = 0; i < sizeof(VkPhysicalDeviceFeatures) >> 2; i++) {
		score += reinterpret_cast<uint32_t*>(&features)[i];
	}

	return score;
}

namespace initium {
	std::expected<VkPhysicalDevice, std::string> pickPhysicalDevice(VkInstance instance, DeviceRequirements requirements, DeviceRequirements optional_requirements) {
		// Get all physical devices
		uint32_t physical_device_count = 0;
		vkEnumeratePhysicalDevices(instance, &physical_device_count, nullptr);
		std::vector<VkPhysicalDevice> physical_devices(physical_device_count);
		vkEnumeratePhysicalDevices(instance, &physical_device_count, physical_devices.data());

		// Get a list of devices that satisfy hard requirements
		std::vector<VkPhysicalDevice> suitable_devices{};
		for (VkPhysicalDevice device : physical_devices) {
			// Test basic supported properties
			if (!areLimitsSatisfied(device, requirements.limits)) { continue; }
			if (!areFeaturesSupported(device, requirements.features)) { continue; }
			if (!areFormatsSupported(device, requirements.formats)) { continue; }

			// Test queue support
			auto supported_families = getQueueFamilies(device);
			bool all_families_supported = true;
			
			for (QueueRequest& requested_family : requirements.queue_requests) {
				bool family_supported = false;
				for (auto& supported_family : supported_families) {
					if ((supported_family.queueFlags & requested_family.flags) == requested_family.flags) {
						family_supported = true;
						break;
					}
				}
				if (!family_supported) { all_families_supported = false; break; }
			}

			if (all_families_supported) { suitable_devices.push_back(device); }
		}

		// Now rank the valid devices
		std::map<int, VkPhysicalDevice> scored_devices{};
		for (VkPhysicalDevice device : suitable_devices) {
			scored_devices[get_score(device)] = device;
		}

		// Return an error if nothing could be found
		if (scored_devices.empty()) {
			return std::unexpected("No suitable devices could be found");
		}

		// We have a device(s), so return the one with the greatest score
		return scored_devices[scored_devices.rbegin()->first];
	}
}


