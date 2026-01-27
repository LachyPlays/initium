#include "allocation.hpp"

#include <bit>
#include <vector>
#include <algorithm>

uint32_t initium::getTypeHeapFlags(initium::MemoryType type) {
	switch (type) {
	case initium::CpuLocalMemory:
		return 0;
	default:
		return VK_MEMORY_HEAP_DEVICE_LOCAL_BIT;
	}
}

uint32_t initium::getRequiredTypeFlags(initium::MemoryType type) {
	switch (type) {
	case initium::CpuLocalMemory:
		return VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	case initium::GpuLocalMemory:
		return VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	case initium::GpuLocalHostVisible:
		return VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
	case initium::GpuLocalHostCoherent:
		return VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	}
}

uint32_t initium::getOptionalTypeFlags(initium::MemoryType type) {
	switch (type) {
	case initium::GpuLocalMemory:
		return 0;
	case initium::CpuLocalMemory:
		return VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
	case initium::GpuLocalHostCoherent:
		return VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
	case initium::GpuLocalHostVisible:
		return VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
	}
}

std::optional<uint32_t> initium::findMemoryTypeIndex(VkPhysicalDevice device, uint32_t type_bits, initium::MemoryType desired_type) {
	VkPhysicalDeviceMemoryProperties mem_props;
	vkGetPhysicalDeviceMemoryProperties(device, &mem_props);

	uint32_t heap_flags = getTypeHeapFlags(desired_type);
	uint32_t heap_index = UINT32_MAX;
	for (int i = 0; i < mem_props.memoryHeapCount; i++) { if ((mem_props.memoryHeaps[i].flags & heap_flags) == heap_flags) heap_index = i; }
	if (heap_index == UINT32_MAX) return std::nullopt;
	
	uint32_t required_type_flags = getRequiredTypeFlags(desired_type);
	uint32_t optional_type_flags = getOptionalTypeFlags(desired_type);
	
	std::vector<std::pair<int, int>> candidates = {};
	for (int i = 0; i < mem_props.memoryTypeCount; i++) {
		if ((1 << i) & type_bits) {
			VkMemoryType potential_type = mem_props.memoryTypes[i];

			if ((potential_type.heapIndex == heap_index) && ((potential_type.propertyFlags & required_type_flags) == required_type_flags)) {
				candidates.push_back(std::make_pair(std::popcount(potential_type.propertyFlags & optional_type_flags), i));
			}
		}
	}

	if (candidates.size() != 0) {
		std::sort(candidates.begin(), candidates.end(), [](std::pair<int, int> a, std::pair<int, int> b) { return a.first > b.first; });
		return candidates[0].second;
	}
	else {
		return std::nullopt;
	}
}

std::optional<VkDeviceMemory> initium::createBufferAllocation(VkDevice device, VkPhysicalDevice phys_device, VkBuffer buffer, initium::MemoryType type) {
	VkMemoryRequirements requirements;
	vkGetBufferMemoryRequirements(device, buffer, &requirements);

	VkMemoryAllocateInfo allocate_info{};
	allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocate_info.allocationSize = requirements.size;
	allocate_info.memoryTypeIndex = initium::findMemoryTypeIndex(phys_device, requirements.memoryTypeBits, type).value();

	VkDeviceMemory memory = VK_NULL_HANDLE;
	if (vkAllocateMemory(device, &allocate_info, nullptr, &memory) != VK_SUCCESS) {
		return std::nullopt;
	}

	return memory;
}

std::optional<VkDeviceMemory> initium::createImageAllocation(VkDevice device, VkPhysicalDevice phys_device, VkImage image, initium::MemoryType type) {
	VkMemoryRequirements requirements;
	vkGetImageMemoryRequirements(device, image, &requirements);

	VkMemoryAllocateInfo allocate_info{};
	allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocate_info.allocationSize = requirements.size;
	allocate_info.memoryTypeIndex = initium::findMemoryTypeIndex(phys_device, requirements.memoryTypeBits, type).value();

	VkDeviceMemory memory = VK_NULL_HANDLE;
	if (vkAllocateMemory(device, &allocate_info, nullptr, &memory) != VK_SUCCESS) {
		return std::nullopt;
	}

	return memory;
}