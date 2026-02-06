#include "framebuffer.hpp"

std::optional<std::vector<VkFramebuffer>> initium::createFramebufferFromViews(VkDevice device, initium::FramebufferParams params) {
	std::vector<VkFramebuffer> framebuffers{};
	
	for (VkImageView& view : params.image_views) {
		std::vector<VkImageView> attachments = {view};
		attachments.insert(attachments.end(), params.attachment_views.begin(), params.attachment_views.end());

		VkFramebufferCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		create_info.renderPass = params.render_pass;
		create_info.attachmentCount = attachments.size();
		create_info.pAttachments = attachments.data();
		create_info.width = params.width;
		create_info.height = params.height;
		create_info.layers = params.layers;

		VkFramebuffer framebuffer = VK_NULL_HANDLE;
		if (vkCreateFramebuffer(device, &create_info, nullptr, &framebuffer) != VK_SUCCESS) {
			return std::nullopt;
		}

		framebuffers.push_back(framebuffer);
	}

	return framebuffers;
}