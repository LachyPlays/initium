#include "command.hpp"

std::optional<VkCommandPool> initium::createCommandPool(VkDevice device, initium::PoolParams params) {
	VkCommandPoolCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	create_info.flags = params.flags;
	create_info.queueFamilyIndex = params.queue_family_indices;

	VkCommandPool command_pool = VK_NULL_HANDLE;
	if (vkCreateCommandPool(device, &create_info, nullptr, &command_pool) != VK_SUCCESS) {
		return std::nullopt;
	}

	return command_pool;
}

std::optional<std::vector<VkCommandBuffer>> initium::allocateCommandBuffer(VkDevice device, VkCommandPool pool, VkCommandBufferLevel level, uint32_t buffer_count) {
	VkCommandBufferAllocateInfo alloc_info{};
	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.commandPool = pool;
	alloc_info.level = level;
	alloc_info.commandBufferCount = buffer_count;

	std::vector<VkCommandBuffer> buffers(buffer_count);
	if (vkAllocateCommandBuffers(device, &alloc_info, buffers.data()) != VK_SUCCESS) {
		return std::nullopt;
	}

	return buffers;
}

VkResult initium::recordCommandBuffer(VkCommandBuffer buffer, VkCommandBufferUsageFlags flags, std::function<void(VkCommandBuffer)> recordFunc) {
	VkCommandBufferBeginInfo begin_info{};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags = flags;
	begin_info.pInheritanceInfo = nullptr;

	if (vkBeginCommandBuffer(buffer, &begin_info) != VK_SUCCESS) {
		return VK_ERROR_UNKNOWN;
	}

	recordFunc(buffer);

	if (vkEndCommandBuffer(buffer) != VK_SUCCESS) {
		return VK_ERROR_UNKNOWN;
	}

	return VK_SUCCESS;
}