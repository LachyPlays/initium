#include "device.hpp"

#include <stdint.h>
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

bool isSwapchainSupported(VkPhysicalDevice device, initium::SwapChainRequest requirement) {
	// Check surface capabilities
	VkSurfaceCapabilitiesKHR capabilities{};
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, requirement.surface, &capabilities);

	// Handle special currentExtent values
	if (capabilities.currentExtent.width != UINT32_MAX) {
		if (capabilities.currentExtent.width != requirement.window_size.width) { return false; }
		if (capabilities.currentExtent.height != requirement.window_size.height) { return false; }
	}

	if (capabilities.maxImageExtent.width < requirement.window_size.width) { return false; }
	if (capabilities.maxImageExtent.height < requirement.window_size.height) { return false; }
	if ((capabilities.supportedCompositeAlpha & requirement.alpha_flags) != requirement.alpha_flags) { return false; }
	if ((capabilities.supportedTransforms & requirement.transform_flags) != requirement.transform_flags) { return false; }
	if ((capabilities.supportedUsageFlags & requirement.usage_flags) != requirement.usage_flags) { return false; }

	// Check format support
	uint32_t format_count = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, requirement.surface, &format_count, nullptr);
	std::vector<VkSurfaceFormatKHR> formats(format_count);
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, requirement.surface, &format_count, formats.data());
	bool match_found = false;
	// Check through all supported formats to see if theres a match
	for (auto format : formats) {
		if ((requirement.format.colorSpace == format.colorSpace) && (requirement.format.format == format.format)) { match_found = true; break; }
	}
	if (!match_found) { return false; }

	// Check present mode support
	uint32_t present_count = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, requirement.surface, &present_count, nullptr);
	std::vector<VkPresentModeKHR> present_modes(present_count);
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, requirement.surface, &present_count, present_modes.data());
	match_found = false;
	// Check through all supported present modes to see if theres a match
	for (auto present_mode : present_modes) {
		if (present_mode == requirement.present_mode) { match_found = true; break; }
	}
	if (!match_found) { return false; }

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

// Returns an array of queue family indices, matching the array of queue requests. If the array size does not match, some requests could not be satisfied. 
std::vector<int> createQueueMapping(VkPhysicalDevice device, std::vector<initium::QueueRequest> queue_requests) {
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
					// !TODO! Figure out how to check if this family supports the requried present surface (if necessary)
	
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

namespace initium {
	std::optional<VkPhysicalDevice> pick_physical_device(VkInstance instance, DeviceRequirements requirements) {
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

			// !TODO! Test swapchain support

			// Test queue support
			auto supported_families = getQueueFamilies(device);
			bool all_families_supported = true;

			for (QueueRequest& requested_family : *requirements.queue_requests) {
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
			return std::nullopt;
		}

		// We have a device(s), so return the one with the greatest score
		return scored_devices[scored_devices.rbegin()->first];
	}

	std::optional<VkDevice> createLogicalDevice(
		VkPhysicalDevice physical_device,
		VkPhysicalDeviceFeatures features,
		std::vector<const char*> extensions,
		std::vector<QueueRequest>& queue_requests,
		std::vector<SwapChainRequest>& swapchain_requests,
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

			queue_requests[i].queue = queue;
			queue_requests[i].family_indice = queue_mapping[i];
		}

		// Swapchain creation
		for (auto& request : swapchain_requests) {
				VkSurfaceCapabilitiesKHR capabilities;
				vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, request.surface, &capabilities);

				// Generate values based upon the surface's capabilities
				uint32_t image_count = std::max(std::min(request.min_surface_count, capabilities.maxImageCount), capabilities.minImageCount); 
				VkExtent2D image_extent = capabilities.currentExtent.width == UINT32_MAX ? request.window_size : capabilities.currentExtent;
				VkSurfaceTransformFlagBitsKHR transform = request.transform_flags == 0 ? capabilities.currentTransform : request.transform_flags;

				VkSwapchainCreateInfoKHR create_info{};
				create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
				create_info.surface = request.surface;
				create_info.minImageCount = image_count;
				create_info.imageFormat = request.format.format;
				create_info.imageColorSpace = request.format.colorSpace;
				create_info.imageExtent = image_extent;
				create_info.imageArrayLayers = 1;
				create_info.imageUsage = request.usage_flags;

				// Handle the different sharing modes

				// Get all the different family indices required by the supported queues
				std::set<uint32_t> indice_set{};
				for (QueueRequest &queue : *request.supported_queues) {
					indice_set.insert(queue.family_indice);
				}
				std::vector<uint32_t> indices(indice_set.begin(), indice_set.end());

				if (indices.size() > 1) {
					create_info.imageSharingMode = request.sharing_mode;
					create_info.queueFamilyIndexCount = indices.size();
					create_info.pQueueFamilyIndices = indices.data();
				}
				else {
					create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
					create_info.queueFamilyIndexCount = 0;
					create_info.pQueueFamilyIndices = nullptr;
				}

				create_info.preTransform = transform;
				create_info.compositeAlpha = request.alpha_flags;
				create_info.presentMode = request.present_mode;
				create_info.clipped = request.clipped;
				create_info.oldSwapchain = VK_NULL_HANDLE;

				VkSwapchainKHR swapchain = VK_NULL_HANDLE;
				if (vkCreateSwapchainKHR(device, &create_info, nullptr, &swapchain) != VK_SUCCESS) {
					return std::nullopt;
				}

				request.swap_chain = swapchain;
			}

		return device;
	}
}
