#ifndef IMAGE_HPP
#define IMAGE_HPP

#include <vulkan/vulkan.h>
#include <optional>
#include <vector>

#include "buffer.hpp"

namespace initium {
	struct ImageViewSwizzles {
		VkComponentSwizzle r = VK_COMPONENT_SWIZZLE_IDENTITY;
		VkComponentSwizzle g = VK_COMPONENT_SWIZZLE_IDENTITY;
		VkComponentSwizzle b = VK_COMPONENT_SWIZZLE_IDENTITY;
		VkComponentSwizzle a = VK_COMPONENT_SWIZZLE_IDENTITY;
	};

	struct ImageViewParams {
		VkImage image;
		VkImageViewType view_type;
		VkFormat format;
		ImageViewSwizzles swizzles = {};
		VkImageAspectFlags aspect_mask;
		uint32_t base_mip_level = 0;
		uint32_t level_count = 1;
		uint32_t base_array_level = 0;
		uint32_t layer_count = 1;
	};

	struct ImageParams {
		uint32_t width, height;
		VkImageType type;
		VkFormat format;
		VkImageTiling tiling;
		VkImageUsageFlags usage;
		std::vector<uint32_t> queue_families;

		VkImageLayout initial_layout = VK_IMAGE_LAYOUT_UNDEFINED;
		VkSampleCountFlagBits samples;
		VkImageCreateFlags flags;
		uint32_t depth = 1;
		uint32_t mip_levels = 1;
		uint32_t array_layers = 1;
	};

	std::optional<VkImageView> create_image_view(VkDevice device, ImageViewParams params);
	std::optional<VkImage> createImage(VkDevice device, ImageParams params);
	std::optional<VkDeviceMemory> createImageAllocation(VkDevice device, VkPhysicalDevice phys_device, VkImage image, initium::MemoryType type);
}

#endif // !IMAGE_HPP
