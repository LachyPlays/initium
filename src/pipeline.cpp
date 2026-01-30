#include "pipeline.hpp"
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

std::optional<VkShaderModule> initium::loadShaderModule(VkDevice device, const char* filename) {
	std::ifstream shader_file(filename, std::ios::binary | std::ios::in | std::ios::ate);

	if (shader_file.is_open()) {
		size_t shader_file_size = (size_t)shader_file.tellg();
		std::vector<char> buffer(shader_file_size);

		shader_file.seekg(0, std::ios::beg);
		shader_file.read(buffer.data(), shader_file_size);
		shader_file.close();

		VkShaderModuleCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		create_info.codeSize = buffer.size();
		create_info.pCode = reinterpret_cast<const uint32_t*>(buffer.data());

		VkShaderModule module = VK_NULL_HANDLE;
		if (vkCreateShaderModule(device, &create_info, nullptr, &module) != VK_SUCCESS) {
			return std::nullopt;
		}

		return module;
	}
	else {
		return std::nullopt;
	}
}

std::optional<VkPipelineLayout> initium::createPipelineLayout(VkDevice device, initium::LayoutParams &params) {
	VkPipelineLayoutCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	create_info.pSetLayouts = params.set_layouts.data();
	create_info.setLayoutCount = params.set_layouts.size();
	create_info.pPushConstantRanges = params.push_ranges.data();
	create_info.pushConstantRangeCount = params.push_ranges.size();

	VkPipelineLayout layout = VK_NULL_HANDLE;
	if (vkCreatePipelineLayout(device, &create_info, nullptr, &layout) != VK_SUCCESS) {
		return std::nullopt;
	}

	return layout;
}

std::optional<VkPipeline> initium::createPipeline(VkDevice device, VkPipelineLayout layout, VkRenderPass pass, initium::PipelineParams &params) {
	// Define shader stages
	std::vector<VkPipelineShaderStageCreateInfo> shader_stages;
	for (initium::ShaderDefine &shader : params.shaders) {
		VkPipelineShaderStageCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		create_info.stage = shader.stage;
		create_info.module = shader.module;
		create_info.pName = shader.entrypoint_name.c_str();

		shader_stages.push_back(create_info);
	}

	// Setup dynamic state
	VkPipelineDynamicStateCreateInfo dynamic_states{};
	dynamic_states.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamic_states.dynamicStateCount = params.dynamics.size();
	dynamic_states.pDynamicStates = params.dynamics.data();

	// Setup vertex input state
	VkPipelineVertexInputStateCreateInfo vertex_inputs{};
	vertex_inputs.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertex_inputs.vertexBindingDescriptionCount = params.vertex_binding_descriptors.size();
	vertex_inputs.pVertexBindingDescriptions = params.vertex_binding_descriptors.data();
	vertex_inputs.vertexAttributeDescriptionCount = params.vertex_attribute_descriptors.size();
	vertex_inputs.pVertexAttributeDescriptions = params.vertex_attribute_descriptors.data();

	// Setup input assembly state
	VkPipelineInputAssemblyStateCreateInfo input_assembly{};
	input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly.topology = params.primitive_topology;
	input_assembly.primitiveRestartEnable = params.primitive_restart;

	// Setup viewport state
	std::vector<VkViewport> viewports{};
	std::vector<VkRect2D> scissors{};
	VkPipelineViewportStateCreateInfo viewport_state{};
	viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	// Viewport check
	{
		if (std::find(params.dynamics.begin(), params.dynamics.end(), VK_DYNAMIC_STATE_VIEWPORT) == params.dynamics.end()) {
			// Dynamic state is not used for the viewport
			for (auto& vp_params : params.viewports) {
				VkViewport viewport{};
				viewport.width = vp_params.width;
				viewport.height = vp_params.height;
				viewport.maxDepth = vp_params.max_depth;
				viewport.minDepth = vp_params.min_depth;
				viewport.x = vp_params.offset_x;
				viewport.y = vp_params.offset_y;

				viewports.push_back(viewport);
			}
			viewport_state.viewportCount = viewports.size();
			viewport_state.pViewports = viewports.data();
		}
		else {
			viewport_state.viewportCount = 1;
			viewport_state.pViewports = nullptr;
		}
		// Scissor check
		if (std::find(params.dynamics.begin(), params.dynamics.end(), VK_DYNAMIC_STATE_SCISSOR) == params.dynamics.end()) {
			// Dynamic state is not used for the scissors
			for (auto& sc_params : params.scissors) {
				VkRect2D scissor{};
				scissor.extent = VkExtent2D{ .width = sc_params.width, .height = sc_params.height };
				scissor.offset = VkOffset2D{ .x = sc_params.offset_x, .y = sc_params.offset_y };

				scissors.push_back(scissor);
			}
			viewport_state.scissorCount = scissors.size();
			viewport_state.pScissors = scissors.data();
		}
		else {
			viewport_state.scissorCount = 1;
			viewport_state.pScissors = nullptr;
		}
	}

	// Setup rasterizer state
	VkPipelineRasterizationStateCreateInfo raster_state{};
	raster_state.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	raster_state.depthClampEnable = params.clamp_depth;
	raster_state.rasterizerDiscardEnable = params.rasterizer_discard;
	raster_state.lineWidth = params.line_width;
	raster_state.cullMode = params.cull_mode;
	raster_state.frontFace = params.front_face;
	raster_state.depthBiasEnable = params.depth_bias_enable;
	raster_state.depthBiasConstantFactor = params.depth_bias_constant_factor;
	raster_state.depthBiasClamp = params.depth_bias_clamp;
	raster_state.depthBiasSlopeFactor = params.depth_bias_slope_factor;

	// Setup multisample state
	VkPipelineMultisampleStateCreateInfo multisample{};
	multisample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisample.sampleShadingEnable = params.msaa_enable;
	multisample.rasterizationSamples = params.msaa_sample_count;
	multisample.minSampleShading = params.msaa_min_sample_shading;
	multisample.pSampleMask = &params.msaa_sample_mask;
	multisample.alphaToCoverageEnable = params.msaa_alpha_to_coverage;
	multisample.alphaToOneEnable = params.msaa_alpha_to_one;

	// In createPipeline function:
	VkPipelineDepthStencilStateCreateInfo depth_stencil{};
	depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depth_stencil.depthTestEnable = params.depth_test_enable;
	depth_stencil.depthWriteEnable = params.depth_write_enable;
	depth_stencil.depthCompareOp = params.depth_compare_op;
	depth_stencil.depthBoundsTestEnable = params.depth_bound_test_enable;
	depth_stencil.maxDepthBounds = params.max_depth_bound;
	depth_stencil.minDepthBounds = params.min_depth_bound;
	depth_stencil.stencilTestEnable = params.stencil_enable;
	depth_stencil.front = params.stencil_front;
	depth_stencil.back = params.stencil_back;

	// Setup attachment blend params
	std::vector<VkPipelineColorBlendAttachmentState> attachment_states{};
	for (auto& attachment : params.attachment_params) {
		VkPipelineColorBlendAttachmentState attachment_state{};
		attachment_state.colorWriteMask = attachment.write_mask;
		attachment_state.blendEnable = attachment.blend_enable;
		attachment_state.srcColorBlendFactor = attachment.src_colour_blend_factor;
		attachment_state.dstColorBlendFactor = attachment.dst_colour_blend_factor;
		attachment_state.colorBlendOp = attachment.colour_blend_op;
		attachment_state.srcAlphaBlendFactor = attachment.src_alpha_blend_factor;
		attachment_state.dstAlphaBlendFactor = attachment.dst_alpha_blend_factor;
		attachment_state.alphaBlendOp = attachment.alpha_blend_op;

		attachment_states.push_back(attachment_state);
	}

	// Setup global blend state
	VkPipelineColorBlendStateCreateInfo blend_state{};
	blend_state.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	blend_state.logicOpEnable = params.blend_logic_op_enable;
	blend_state.logicOp = params.blend_logic_op;
	blend_state.attachmentCount = attachment_states.size();
	blend_state.pAttachments = attachment_states.data();
	blend_state.blendConstants[0] = params.blend_constants[0];
	blend_state.blendConstants[1] = params.blend_constants[1];
	blend_state.blendConstants[2] = params.blend_constants[2];
	blend_state.blendConstants[3] = params.blend_constants[3];

	// Setup pipeline creation state
	VkGraphicsPipelineCreateInfo pipeline_info{};
	pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_info.stageCount = shader_stages.size();
	pipeline_info.pStages = shader_stages.data();
	pipeline_info.pDynamicState = &dynamic_states;
	pipeline_info.pVertexInputState = &vertex_inputs;
	pipeline_info.pInputAssemblyState = &input_assembly;
	pipeline_info.pViewportState = &viewport_state;
	pipeline_info.pRasterizationState = &raster_state;
	pipeline_info.pMultisampleState = &multisample;
	pipeline_info.pDepthStencilState = &depth_stencil;
	pipeline_info.pColorBlendState = &blend_state;
	pipeline_info.layout = layout;
	pipeline_info.renderPass = pass;
	pipeline_info.subpass = params.subpass_index;
	pipeline_info.basePipelineHandle = params.base_pipeline;
	pipeline_info.basePipelineIndex = params.base_pipeline_index;


	VkPipeline pipeline = VK_NULL_HANDLE;
	if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &pipeline) != VK_SUCCESS) {
		return std::nullopt;
	}

	return pipeline;
}