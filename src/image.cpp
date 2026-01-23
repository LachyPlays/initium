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