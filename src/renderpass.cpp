#include "renderpass.hpp"

#include <optional>
#include <vulkan/vulkan.h>

std::optional<VkRenderPass> initium::createRenderPass(VkDevice device, initium::RenderPassParams params) {
	// Setup attachment descriptors
	std::vector<VkAttachmentDescription> attachment_descriptors{};
	for (auto &attachment : params.attachments) {
		VkAttachmentDescription descriptor{};
		descriptor.format = attachment.format;
		descriptor.samples = attachment.samples;
		descriptor.loadOp = attachment.load_op;
		descriptor.storeOp = attachment.store_op;
		descriptor.stencilLoadOp = attachment.stencil_load_op;
		descriptor.stencilStoreOp = attachment.stencil_store_op;
		descriptor.initialLayout = attachment.initial_layout;
		descriptor.finalLayout = attachment.final_layout;

		attachment_descriptors.push_back(descriptor);
	} 

	// Setup subpass descriptors
	std::vector<VkSubpassDescription> subpass_descriptors{};
	for (auto &subpass : params.subpasses) {
		VkSubpassDescription description{};
		description.pipelineBindPoint = subpass.bind_point;
		description.colorAttachmentCount = subpass.colour_references.size();
		description.pColorAttachments = subpass.colour_references.data();
		description.pDepthStencilAttachment = &subpass.depth_attachment_ref;

		subpass_descriptors.push_back(description);
	}
	
	VkRenderPassCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	create_info.attachmentCount = attachment_descriptors.size();
	create_info.pAttachments = attachment_descriptors.data();
	create_info.subpassCount = subpass_descriptors.size();
	create_info.pSubpasses = subpass_descriptors.data();
	create_info.dependencyCount = params.dependencies.size();
	create_info.pDependencies = params.dependencies.data();

	VkRenderPass render_pass = VK_NULL_HANDLE;
	if (vkCreateRenderPass(device, &create_info, nullptr, &render_pass) != VK_SUCCESS) {
		return std::nullopt;
	}

	return render_pass;
}