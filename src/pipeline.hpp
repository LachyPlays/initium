#ifndef PIPELINE_HPP
#define PIPELINE_HPP

#include <vulkan/vulkan.h>
#include <optional>
#include <string>
#include <vector>

namespace initium {
	struct ShaderDefine {
		VkShaderModule module;
		VkShaderStageFlagBits stage;
		std::string entrypoint_name;
	};

	struct ViewportParams {
		float offset_x = 0.0f;
		float offset_y = 0.0f;
		float width;
		float height;
		float min_depth = 0.0f;
		float max_depth = 1.0f;

		VkViewport toViewport() {
			VkViewport viewport{};
			viewport.x = this->offset_x;
			viewport.y = this->offset_y;
			viewport.width = this->width;
			viewport.height = this->height;
			viewport.minDepth = this->min_depth;
			viewport.maxDepth = this->max_depth;

			return viewport;
		}
	};

	struct ScissorParams {
		int offset_x = 0;
		int offset_y = 0;
		unsigned int width;
		unsigned int height;

		VkRect2D toRect2D() {
			VkRect2D rect{};
			rect.offset = VkOffset2D{ .x = this->offset_x, .y = this->offset_y };
			rect.extent = VkExtent2D{ .width = this->width, .height = this->height };

			return rect;
		}
	};

	struct AttachmentParams {
		// Descriptor parameters

		VkFormat format;
		VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
		VkAttachmentLoadOp load_op;
		VkAttachmentStoreOp store_op;
		VkAttachmentLoadOp stencil_load_op = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		VkAttachmentStoreOp stencil_store_op = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		VkImageLayout initial_layout;
		VkImageLayout final_layout;


		// Blending parameters

		VkColorComponentFlags write_mask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		VkBool32 blend_enable = VK_FALSE;
		VkBlendFactor src_colour_blend_factor;
		VkBlendFactor dst_colour_blend_factor;
		VkBlendOp colour_blend_op;
		VkBlendFactor src_alpha_blend_factor;
		VkBlendFactor dst_alpha_blend_factor;
		VkBlendOp alpha_blend_op;
	};

	struct LayoutParams {
		std::vector<VkDescriptorSetLayout> set_layouts = {};
		std::vector<VkPushConstantRange> push_ranges = {};
	};


	struct PipelineParams {
		// Globally applicable parameters
		std::vector<ShaderDefine> shaders;
		std::vector<VkDynamicState> dynamics = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		uint32_t subpass_index = 0;

		// Base-pipeline parameters
		VkPipeline base_pipeline = VK_NULL_HANDLE;
		uint32_t base_pipeline_index = (uint32_t)-1;

		// Vertex-related parameters
		std::vector<VkVertexInputBindingDescription> vertex_binding_descriptors;
		std::vector<VkVertexInputAttributeDescription> vertex_attribute_descriptors;
		VkPrimitiveTopology primitive_topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		VkBool32 primitive_restart = VK_FALSE;
		
		// Scissor & viewport parameters
		std::vector<ViewportParams> viewports = {};
		std::vector<ScissorParams> scissors = {};

		// Rasterizer parameters
		VkBool32 clamp_depth = VK_FALSE;
		VkBool32 rasterizer_discard = VK_FALSE;
		VkPolygonMode polygon_mode = VK_POLYGON_MODE_FILL;
		float line_width = 1.0f;
		VkCullModeFlagBits cull_mode = VK_CULL_MODE_NONE;
		VkFrontFace front_face = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		VkBool32 depth_bias_enable = VK_FALSE;
		float depth_bias_constant_factor = 0.0f;
		float depth_bias_clamp = 0.0f;
		float depth_bias_slope_factor = 0.0f;

		// Multisampling parameters
		VkBool32 msaa_enable = VK_FALSE;
		VkSampleCountFlagBits msaa_sample_count = VK_SAMPLE_COUNT_1_BIT;
		float msaa_min_sample_shading = 1.0f;
		VkSampleMask msaa_sample_mask = UINT32_MAX;
		VkBool32 msaa_alpha_to_coverage = VK_FALSE;
		VkBool32 msaa_alpha_to_one = VK_FALSE;

		// Depth stencil parameters
		VkBool32 depth_test_enable = VK_FALSE;
		VkBool32 depth_write_enable = VK_FALSE;
		VkCompareOp depth_compare_op = VK_COMPARE_OP_LESS;
		VkBool32 depth_bound_test_enable = VK_FALSE;
		float max_depth_bound = 1.0f;
		float min_depth_bound = 0.0f;
		VkBool32 stencil_enable = VK_FALSE;
		VkStencilOpState stencil_front = {};
		VkStencilOpState stencil_back = {};

		// Attachment parameters (ORDER DEPENDANT)
		std::vector<AttachmentParams> attachment_params;

		// Global blending parameters 
		VkBool32 blend_logic_op_enable = VK_FALSE;
		VkLogicOp blend_logic_op = VK_LOGIC_OP_COPY;
		float blend_constants[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	};

	std::optional<VkShaderModule> loadShaderModule(VkDevice device, const char* filename);
	std::optional<VkPipelineLayout> createPipelineLayout(VkDevice device, LayoutParams &params);
	std::optional<VkPipeline> createPipeline(VkDevice device, VkPipelineLayout layout, VkRenderPass pass, PipelineParams &params);
}

#endif