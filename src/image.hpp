#ifndef IMAGE_HPP
#define IMAGE_HPP

#include <vulkan/vulkan.h>
#include <optional>

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

	std::optional<VkImageView> create_image_view(VkDevice device, ImageViewParams params);
}

#endif // !IMAGE_HPP
