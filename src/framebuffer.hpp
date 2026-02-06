#ifndef FRAMEBUFFER_HPP
#define FRAMEBUFFER_HPP

#include <vulkan/vulkan.h>
#include <optional>
#include <vector>

namespace initium {
	struct FramebufferParams {
		std::vector<VkImageView> image_views;
		std::vector<VkImageView> attachment_views = {};

		VkRenderPass render_pass;

		int width, height;
		int layers = 1;
	};

	std::optional<std::vector<VkFramebuffer>> createFramebufferFromViews(VkDevice device, FramebufferParams params);
};

#endif