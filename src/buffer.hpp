#ifndef BUFFER_HPP
#define BUFFER_HPP

#include <vulkan/vulkan.h>
#include <optional>
#include <vector>

#include "allocation.hpp"

namespace initium {
	struct BufferParams {
		VkBufferCreateFlags flags = 0;
		size_t size;
		VkBufferUsageFlags usage;
		VkSharingMode sharing_mode = VK_SHARING_MODE_EXCLUSIVE;
		std::vector<uint32_t> concurrent_queue_family_indices = {};
	};

	std::optional<VkBuffer> createBuffer(VkDevice device, BufferParams& params);
	std::optional<VkDeviceMemory> createBufferAllocation(VkDevice device, VkPhysicalDevice phys_device, VkBuffer buffer, initium::MemoryType type);
};

#endif