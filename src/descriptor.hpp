#ifndef DESCRIPTOR_HPP
#define DESCRIPTOR_HPP

#include <vulkan/vulkan.h>
#include <optional>
#include <vector>

namespace initium {
    std::optional<VkDescriptorSetLayout> createDescriptorLayout(VkDevice device, std::vector<VkDescriptorSetLayoutBinding> bindings);
    std::optional<VkDescriptorPool> createDescriptorPool(VkDevice device, std::vector<VkDescriptorPoolSize> set_types, size_t set_count);
    std::optional<std::vector<VkDescriptorSet>> allocateDescriptorSets(VkDevice device, VkDescriptorPool pool, std::vector<VkDescriptorSetLayout> layouts, size_t set_count);
};

#endif