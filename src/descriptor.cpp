#include "descriptor.hpp"

std::optional<VkDescriptorPool> initium::createDescriptorPool(VkDevice device, std::vector<VkDescriptorPoolSize> set_types, size_t set_count) {
    VkDescriptorPoolCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    create_info.pPoolSizes = set_types.data();
    create_info.poolSizeCount = set_types.size();
    create_info.maxSets = set_count;

    VkDescriptorPool pool = VK_NULL_HANDLE;
    if (vkCreateDescriptorPool(device, &create_info, nullptr, &pool) != VK_SUCCESS) {
        return std::nullopt;
    }

    return pool;
}

std::optional<std::vector<VkDescriptorSet>> initium::allocateDescriptorSets(VkDevice device, VkDescriptorPool pool, std::vector<VkDescriptorSetLayout> layouts, size_t set_count) {
    VkDescriptorSetAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = pool;
    alloc_info.descriptorSetCount = set_count;
    alloc_info.pSetLayouts = layouts.data();

    std::vector<VkDescriptorSet> sets(set_count);
    if (vkAllocateDescriptorSets(device, &alloc_info, sets.data()) != VK_SUCCESS) {
        return std::nullopt;
    }

    return sets;
}

std::optional<VkDescriptorSetLayout> initium::createDescriptorLayout(VkDevice device, std::vector<VkDescriptorSetLayoutBinding> bindings) {
    VkDescriptorSetLayoutCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    create_info.bindingCount = bindings.size();
    create_info.pBindings = bindings.data();

    VkDescriptorSetLayout layout = VK_NULL_HANDLE;
    if (vkCreateDescriptorSetLayout(device, &create_info, nullptr, &layout) != VK_SUCCESS) {
        return std::nullopt;
    }

    return layout;
}