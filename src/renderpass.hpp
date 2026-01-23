#ifndef RENDERPASS_HPP
#define RENDERPASS_HPP

#include "pipeline.hpp"

namespace initium {
	struct SubpassParams {
		VkPipelineBindPoint bind_point;
		std::vector<VkAttachmentReference> colour_references;
	};

	struct RenderPassParams {
		std::vector<SubpassParams> subpasses;
		std::vector<AttachmentParams> attachments;
		std::vector<VkSubpassDependency> dependencies;
	};

	std::optional<VkRenderPass> createRenderPass(VkDevice device, RenderPassParams &params);
};

#endif