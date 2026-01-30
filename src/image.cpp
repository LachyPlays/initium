#include "image.hpp"

std::optional<VkImageView> initium::create_image_view(VkDevice device, initium::ImageViewParams params) {
	VkImageViewCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	create_info.image = params.image;
	create_info.viewType = params.view_type;
	create_info.format = params.format;

	create_info.components.r = params.swizzles.r;
	create_info.components.g = params.swizzles.g;
	create_info.components.b = params.swizzles.b;
	create_info.components.a = params.swizzles.a;

	create_info.subresourceRange.aspectMask = params.aspect_mask;
	create_info.subresourceRange.baseMipLevel = params.base_mip_level;
	create_info.subresourceRange.levelCount = params.level_count;
	create_info.subresourceRange.baseArrayLayer = params.base_array_level;
	create_info.subresourceRange.layerCount = params.layer_count;

	VkImageView image_view = VK_NULL_HANDLE;
	if (vkCreateImageView(device, &create_info, nullptr, &image_view) != VK_SUCCESS) {
		return std::nullopt;
	}

	return image_view;
}

std::optional<VkImage> initium::createImage(VkDevice device, initium::ImageParams params) {
	VkImageCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	create_info.extent = VkExtent3D(params.width, params.height, params.depth);
	create_info.imageType = params.type;
	create_info.format = params.format;
	create_info.tiling = params.tiling;
	create_info.usage = params.usage;
	create_info.initialLayout = params.initial_layout;
	create_info.samples = params.samples;
	create_info.flags = params.flags;
	create_info.mipLevels = params.mip_levels;
	create_info.arrayLayers = params.array_layers;
	
	if (params.queue_families.size() <= 1) {
		create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		create_info.pQueueFamilyIndices = nullptr;
		create_info.queueFamilyIndexCount = 0;
	} else {
		create_info.sharingMode = VK_SHARING_MODE_CONCURRENT;
		create_info.pQueueFamilyIndices = params.queue_families.data();
		create_info.queueFamilyIndexCount = params.queue_families.size();
	}

	VkImage image = VK_NULL_HANDLE;
	if (vkCreateImage(device, &create_info, nullptr, &image) != VK_SUCCESS) {
		return std::nullopt;
	}

	return image;
}

std::optional<VkSampler> initium::createSampler(VkDevice device, initium::SamplerParams params) {
	VkSamplerCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	create_info.magFilter = params.mag_filter;
	create_info.minFilter = params.min_filter;
	create_info.addressModeU = params.mode_u;
	create_info.addressModeV = params.mode_v;
	create_info.addressModeW = params.mode_w;
	create_info.borderColor = params.border_colour;
	create_info.unnormalizedCoordinates = params.unnormalized_coords;
	create_info.compareEnable = params.compare_enable;
	create_info.compareOp = params.compare_op;
	create_info.mipmapMode = params.mipmap_mode;
	create_info.mipLodBias = params.mip_lod_bias;
	create_info.minLod = params.min_lod;
	create_info.maxLod = params.max_lod;

	if (params.anistrophy_sample_limit > 0.0f) {
		create_info.anisotropyEnable = VK_TRUE;
		create_info.maxAnisotropy = params.anistrophy_sample_limit;
	} else {
		create_info.anisotropyEnable = VK_FALSE;
	}

	VkSampler sampler = VK_NULL_HANDLE;
	if (vkCreateSampler(device, &create_info, nullptr, &sampler) != VK_SUCCESS) {
		return std::nullopt;
	}

	return sampler;
}