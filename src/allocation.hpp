#ifndef ALLOCATION_HPP
#define ALLOCATION_HPP

#include <vulkan/vulkan.h>
#include <optional>

namespace initium {
    enum MemoryType {
		CpuLocalMemory,
		GpuLocalMemory,

		GpuLocalHostVisible,
		GpuLocalHostCoherent,
	};

    uint32_t getTypeHeapFlags(MemoryType type);
    uint32_t getRequiredTypeFlags(MemoryType type);
    uint32_t getOptionalTypeFlags(MemoryType type);
    std::optional<uint32_t> findMemoryTypeIndex(VkPhysicalDevice device, uint32_t type_bits, MemoryType desired_type);
    std::optional<VkDeviceMemory> createBufferAllocation(VkDevice device, VkPhysicalDevice phys_device, VkBuffer buffer, MemoryType type);
    std::optional<VkDeviceMemory> createImageAllocation(VkDevice device, VkPhysicalDevice phys_device, VkImage image, MemoryType type);
};

#endif