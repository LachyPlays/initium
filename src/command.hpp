#ifndef COMMAND_HPP
#define COMMAND_HPP

#include <vulkan/vulkan.h>
#include <optional>
#include <vector>
#include <functional>

namespace initium {
	struct PoolParams {
		VkCommandPoolCreateFlags flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		uint32_t queue_family_indices;
	};

	std::optional<VkCommandPool> createCommandPool(VkDevice device, PoolParams& params);
	
	std::optional<std::vector<VkCommandBuffer>> allocateCommandBuffer(VkDevice device, VkCommandPool pool, VkCommandBufferLevel level, uint32_t buffer_count);
	VkResult recordCommandBuffer(VkCommandBuffer buffer, VkCommandBufferUsageFlags flags, std::function<void(VkCommandBuffer)> recordFunc);
};

#endif