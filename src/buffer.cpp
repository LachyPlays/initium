#include "buffer.hpp"

std::optional<VkBuffer> initium::createBuffer(VkDevice device, initium::BufferParams params) {
	VkBufferCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	create_info.flags = params.flags;
	create_info.size = params.size;
	create_info.usage = params.usage;
	create_info.sharingMode = params.sharing_mode;
	create_info.queueFamilyIndexCount = params.concurrent_queue_family_indices.size();
	create_info.pQueueFamilyIndices = params.concurrent_queue_family_indices.data();

	VkBuffer buffer = VK_NULL_HANDLE;
	if (vkCreateBuffer(device, &create_info, nullptr, &buffer) != VK_SUCCESS) {
		return std::nullopt;
	}

	return buffer;
}