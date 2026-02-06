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
		int width, height;
		VkImageType type;
		VkFormat format;
		VkImageTiling tiling;
		VkImageUsageFlags usage;
		std::vector<uint32_t> queue_families;

		VkImageLayout initial_layout = VK_IMAGE_LAYOUT_UNDEFINED;
		VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
		VkImageCreateFlags flags = 0;
		uint32_t depth = 1;
		uint32_t mip_levels = 1;
		uint32_t array_layers = 1;
	};

	struct SamplerParams {
		VkFilter mag_filter;
		VkFilter min_filter;

		VkSamplerAddressMode mode_u = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		VkSamplerAddressMode mode_v = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		VkSamplerAddressMode mode_w = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		float anistrophy_sample_limit = 0.0f;
		VkBorderColor border_colour = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		VkBool32 unnormalized_coords = VK_FALSE;
		VkBool32 compare_enable = VK_FALSE;
		VkCompareOp compare_op = VK_COMPARE_OP_ALWAYS;
		VkSamplerMipmapMode mipmap_mode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		float mip_lod_bias = 0.0f;
		float min_lod = 0.0f;
		float max_lod = 0.0f;
	};

	std::optional<VkImageView> createImageView(VkDevice device, ImageViewParams params);
	std::optional<VkImage> createImage(VkDevice device, ImageParams params);
	std::optional<VkDeviceMemory> createImageAllocation(VkDevice device, VkPhysicalDevice phys_device, VkImage image, initium::MemoryType type);
	std::optional<VkSampler> createSampler(VkDevice device, SamplerParams params);
}

#endif // !IMAGE_HPP
