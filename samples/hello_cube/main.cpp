#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_VULKAN
#include<GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "instance.hpp"
#include "device.hpp"
#include "image.hpp"
#include "pipeline.hpp"
#include "renderpass.hpp"
#include "framebuffer.hpp"
#include "command.hpp"
#include "buffer.hpp"
#include "descriptor.hpp"

#include <vector>
#include <iostream>

#define FRAMEBUFFER_FORMAT VK_FORMAT_R8G8B8A8_SRGB

int main() {
	// Initialize GLFW
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	GLFWwindow* window = glfwCreateWindow(600, 400, "Hello cube", NULL, NULL);

	uint32_t glfw_extension_count = 0;
	const char** glfw_extensions_raw = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
	std::vector<const char*> extensions(glfw_extension_count);
	for (unsigned int i = 0; i < glfw_extension_count; i++) { extensions[i] = glfw_extensions_raw[i]; };
	extensions.push_back("VK_EXT_debug_utils");

	// Initialize renderer
	const unsigned int max_frames_in_flight = 2;

	auto instance_result = initium::createInstance(
		{ .application_name = "Hello cube",
			.application_version = {1, 0, 0},
			.extensions = extensions,
			.enable_validation_layers = true
		});
	if (!instance_result.has_value()) {
		printf("Failed to create instance\n");
		return 1;
	}
	VkInstance instance = instance_result.value().instance;
	VkDebugUtilsMessengerEXT debug_messenger = instance_result.value().debug_messenger;

	// Surface creation
	VkSurfaceKHR surface = VK_NULL_HANDLE;
	if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
		std::cerr << "Failed to create window surface" << std::endl;
		return 1;
	}
	int surface_width, surface_height;
	glfwGetFramebufferSize(window, &surface_width, &surface_height);

	// Swapchain request
	VkSurfaceFormatKHR swapchain_format = { .format = VK_FORMAT_R8G8B8A8_SRGB, .colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR };
	uint32_t surface_count = 3;

	// Device queues
	std::vector<initium::QueueRequest> queue_requests = {
			{.flags = VK_QUEUE_GRAPHICS_BIT},
			{.flags = VK_QUEUE_TRANSFER_BIT}
	};

	std::vector<initium::SwapChainRequest> swapchain_requests = { {
	.supported_queues = &queue_requests,
	.surface = surface,
	.window_size = VkExtent2D(surface_width, surface_height),
	.format = swapchain_format,
	.present_mode = VK_PRESENT_MODE_MAILBOX_KHR,
	.surface_count = surface_count,
	.usage_flags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
	} };


	auto physical_device = initium::pick_physical_device(instance,
		{ .queue_requests = &queue_requests,
		 .formats = {},
		 .features = {},
		 .limits = {},
		 .extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME}
		}).value();

	auto device_result = initium::createLogicalDevice(
		physical_device,
		{},
		{ VK_KHR_SWAPCHAIN_EXTENSION_NAME },
		queue_requests,
		swapchain_requests,
		true
	);
	auto device = device_result.value();
	if (!queue_requests[0].queue.has_value()) { std::cerr << "Failed to create graphics queue" << std::endl; return 1; }
	if (!queue_requests[1].queue.has_value()) { std::cerr << "Failed to create transfer queue" << std::endl; return 1; }
	auto graphics_queue = queue_requests[0].queue.value();
	auto transfer_queue = queue_requests[1].queue.value();

	if (!swapchain_requests[0].swap_chain.has_value()) { std::cerr << "Failed to create swapchain" << std::endl; return 1; }
	auto swapchain = swapchain_requests[0].swap_chain.value();

	// Swapchain images
	std::vector<VkImage> swapchain_images{};
	uint32_t swapchain_image_count = 0;
	vkGetSwapchainImagesKHR(device, swapchain, &swapchain_image_count, nullptr);
	if (swapchain_image_count != surface_count) { std::cerr << "Number of swapchain surfaces does not match requested"; return 1; }
	swapchain_images.resize(swapchain_image_count);
	vkGetSwapchainImagesKHR(device, swapchain, &swapchain_image_count, swapchain_images.data());

	std::vector<VkImageView> swapchain_views(swapchain_image_count);
	for (int i = 0; i < swapchain_images.size(); i++) {
		auto image_view_result = initium::create_image_view(device, {
			.image = swapchain_images[i],
			.view_type = VK_IMAGE_VIEW_TYPE_2D,
			.format = FRAMEBUFFER_FORMAT,
			.aspect_mask = VK_IMAGE_ASPECT_COLOR_BIT,

			});

		if (image_view_result.has_value()) {
			swapchain_views[i] = image_view_result.value();
		}
		else {
			std::cerr << "Failed to create a swapchain image view";
			return 1;
		}
	}

	// Shaders
	auto vert_shader_result = initium::loadShaderModule(device, "shaders/shader.vert.spv");
	if (!vert_shader_result.has_value()) {
		std::cerr << "Failed to load vertex shader"; return 1;
	}
	auto frag_shader_result = initium::loadShaderModule(device, "shaders/shader.frag.spv");
	if (!frag_shader_result.has_value()) {
		std::cerr << "Failed to load fragment shader"; return 1;
	}
	VkShaderModule vert_shader = vert_shader_result.value();
	VkShaderModule frag_shader = frag_shader_result.value();

	initium::ShaderDefine vert_define = {
		.module = vert_shader,
		.stage = VK_SHADER_STAGE_VERTEX_BIT,
		.entrypoint_name = "main"
	};
	initium::ShaderDefine frag_define = {
		.module = frag_shader,
		.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
		.entrypoint_name = "main"
	};

	// Attachments
	initium::AttachmentParams fb_attachment = {
		.format = FRAMEBUFFER_FORMAT,
		.load_op = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.store_op = VK_ATTACHMENT_STORE_OP_STORE,
		.initial_layout = VK_IMAGE_LAYOUT_UNDEFINED,
		.final_layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
	};
	VkAttachmentReference fb_reference{};
	fb_reference.attachment = 0;
	fb_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	// Subpass dependencies
	VkSubpassDependency subpass_dependency{};
	subpass_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	subpass_dependency.dstSubpass = 0;
	subpass_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpass_dependency.srcAccessMask = 0;
	subpass_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpass_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	// Render passes
	initium::SubpassParams main_subpass = {
		.bind_point = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.colour_references = { fb_reference }
	};

	initium::RenderPassParams pass_params = {
			.subpasses = { main_subpass },
			.attachments = { fb_attachment },
			.dependencies = { subpass_dependency }
	};

	auto pass_result = initium::createRenderPass(device, pass_params);
	if (!pass_result.has_value()) {
		printf("Failed to create render pass\n");
		return 1;
	}
	VkRenderPass pass = pass_result.value();

	// Create transfer buffer
	initium::PoolParams transfer_pool_params = { 
		.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT, 
		.queue_family_indices = queue_requests[0].family_indice };
	VkCommandPool transfer_pool = initium::createCommandPool(device, transfer_pool_params).value();
	VkCommandBuffer transfer_buffer = initium::allocateCommandBuffer(device, transfer_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1).value()[0];

	// Vertex feeding
	typedef struct {
		glm::vec3 pos;
		glm::vec3 colour;
	} Vertex;

	const std::vector<Vertex> vertices = {
		{{ -0.5f, -0.5f, -0.5f }, {1.0f, 0.0f, 0.0f }}, // 0 Front Top left
		{{  0.5f, -0.5f, -0.5f }, {0.0f, 1.0f, 0.0f }}, // 1 Front Top right
		{{ -0.5f,  0.5f, -0.5f }, {0.0f, 0.0f, 1.0f }}, // 2 Front Bottom left
		{{  0.5f,  0.5f, -0.5f }, {1.0f, 1.0f, 0.0f }}, // 3 Front Bottom right
		{{ -0.5f, -0.5f,  0.5f }, {1.0f, 0.0f, 0.0f }}, // 4 Back Top left
		{{  0.5f, -0.5f,  0.5f }, {0.0f, 1.0f, 0.0f }}, // 5 Back Top right
		{{ -0.5f,  0.5f,  0.5f }, {0.0f, 0.0f, 1.0f }}, // 6 Back Bottom left
		{{  0.5f,  0.5f,  0.5f }, {1.0f, 1.0f, 0.0f }}  // 7 Back Bottom right
	};
	size_t vertice_bytes = sizeof(Vertex) * vertices.size();

	const std::vector<uint16_t> indices = {
    // Front face (z = -0.5)
    0, 2, 1,
    1, 2, 3,
    // Back face (z = +0.5)
    5, 7, 4,
    4, 7, 6,
    // Left face (x = -0.5)
    4, 6, 0,
    0, 6, 2,
    // Right face (x = +0.5)
    1, 3, 5,
    5, 3, 7,
    // Top face (y = -0.5)
    4, 0, 5,
    5, 0, 1,
    // Bottom face (y = +0.5)
    2, 6, 3,
    3, 6, 7
};

	size_t indice_bytes = sizeof(uint32_t) * indices.size();

	size_t staging_bytes = vertice_bytes + indice_bytes;

	initium::BufferParams vertex_buffer_params = { .size = vertice_bytes, .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT };
	VkBuffer vertex_buffer = initium::createBuffer(device, vertex_buffer_params).value();
	VkDeviceMemory vertex_memory = initium::createBufferAllocation(device, physical_device, vertex_buffer, initium::GpuLocalMemory).value();
	vkBindBufferMemory(device, vertex_buffer, vertex_memory, 0);

	initium::BufferParams index_buffer_params = { .size = indice_bytes, .usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT};
	VkBuffer index_buffer = initium::createBuffer(device, index_buffer_params).value();
	VkDeviceMemory index_memory = initium::createBufferAllocation(device, physical_device, index_buffer, initium::GpuLocalMemory).value();
	vkBindBufferMemory(device, index_buffer, index_memory, 0);

	initium::BufferParams staging_buffer_params = { .size = staging_bytes, .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT };
	VkBuffer staging_buffer = initium::createBuffer(device, staging_buffer_params).value();
	VkDeviceMemory staging_memory = initium::createBufferAllocation(device, physical_device, staging_buffer, initium::GpuLocalHostCoherent).value();
	vkBindBufferMemory(device, staging_buffer, staging_memory, 0);

	void* staging_data = nullptr;
	vkMapMemory(device, staging_memory, 0, staging_bytes, 0, &staging_data);
	memcpy(staging_data, vertices.data(), vertice_bytes);
	memcpy((uint8_t*)staging_data + vertice_bytes, indices.data(), indice_bytes);
	vkUnmapMemory(device, staging_memory);

	if (initium::recordCommandBuffer(transfer_buffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
		[vertex_buffer, vertices, vertice_bytes, index_buffer, indices, indice_bytes, staging_buffer](VkCommandBuffer command_buffer) {
		VkBufferCopy vertex_region = { .srcOffset = 0, .dstOffset = 0, .size = vertice_bytes };
		vkCmdCopyBuffer(command_buffer, staging_buffer, vertex_buffer, 1, &vertex_region);
		VkBufferCopy index_region = { .srcOffset = vertice_bytes, .dstOffset = 0, .size = indice_bytes};
		vkCmdCopyBuffer(command_buffer, staging_buffer, index_buffer, 1, &index_region);
	}) != VK_SUCCESS) {
		printf("Failed to record transfer buffer\n");
		return 1;
	}

	VkSubmitInfo transfer_submit{};
	transfer_submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	transfer_submit.commandBufferCount = 1;
	transfer_submit.pCommandBuffers = &transfer_buffer;
	vkQueueSubmit(graphics_queue, 1, &transfer_submit, VK_NULL_HANDLE);
	vkQueueWaitIdle(graphics_queue);
	vkFreeCommandBuffers(device, transfer_pool, 1, &transfer_buffer);
	vkDestroyCommandPool(device, transfer_pool, nullptr);
	vkDestroyBuffer(device, staging_buffer, nullptr);
	vkFreeMemory(device, staging_memory, nullptr);

	// Vertex description

	VkVertexInputBindingDescription binding_descriptors{};
	binding_descriptors.binding = 0;
	binding_descriptors.stride = sizeof(Vertex);
	binding_descriptors.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	std::vector<VkVertexInputAttributeDescription> attribute_descriptors(2);
	attribute_descriptors[0].binding = 0;
	attribute_descriptors[0].location = 0;
	attribute_descriptors[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attribute_descriptors[0].offset = offsetof(Vertex, pos);

	attribute_descriptors[1].binding = 0;
	attribute_descriptors[1].location = 1;
	attribute_descriptors[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attribute_descriptors[1].offset = offsetof(Vertex, colour);


	// Descriptors
	struct BindingUniformObject {
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 proj;
		glm::mat4 mvp;
	};

	std::vector<VkDescriptorSetLayoutBinding> layout_bindings;

	VkDescriptorSetLayoutBinding set_binding{};
	set_binding.binding = 0;
	set_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	set_binding.descriptorCount = 1;
	set_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	set_binding.pImmutableSamplers = nullptr;

	layout_bindings.push_back(set_binding);

	auto set_layout_result = initium::createDescriptorLayout(device, layout_bindings);
	if (!set_layout_result.has_value()) {
		printf("Failed to create desciptor set layout\n");
		return 1;
	}
	VkDescriptorSetLayout set_layout = set_layout_result.value();
	std::vector<VkDescriptorSetLayout> set_layouts(max_frames_in_flight, set_layout);

	std::vector<VkBuffer> binding_uniform_buffers(max_frames_in_flight);
	std::vector<VkDeviceMemory> binding_uniform_memory(max_frames_in_flight);
	std::vector<void*> binding_buffer_mappings(max_frames_in_flight);

	for	(int i = 0; i < max_frames_in_flight; i++) {
		initium::BufferParams buffer_params = {
			.size = sizeof(BindingUniformObject),
			.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		};

		binding_uniform_buffers[i] = initium::createBuffer(device, buffer_params).value();
		binding_uniform_memory[i] = initium::createBufferAllocation(device, physical_device, binding_uniform_buffers[i], initium::GpuLocalHostCoherent).value();
		vkBindBufferMemory(device, binding_uniform_buffers[i], binding_uniform_memory[i], 0);

		vkMapMemory(device, binding_uniform_memory[i], 0, sizeof(BindingUniformObject), 0, &binding_buffer_mappings[i]);
	}

	VkDescriptorPoolSize descriptor_pool_size{};
	descriptor_pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptor_pool_size.descriptorCount = max_frames_in_flight;

	std::vector<VkDescriptorPoolSize> pool_sizes{descriptor_pool_size};

	auto descriptor_pool_result = initium::createDescriptorPool(device, pool_sizes, max_frames_in_flight);
	if (!descriptor_pool_result.has_value()) {
		printf("Failed to create descriptor pool\n");
		return 1;
	}
	VkDescriptorPool descriptor_pool = descriptor_pool_result.value();

	auto descriptor_sets_result = initium::allocateDescriptorSets(device, descriptor_pool, set_layouts, max_frames_in_flight);
	if (!descriptor_sets_result.has_value()) {
		printf("Failed to allocate descriptor sets\n");
		return 1;
	}
	std::vector<VkDescriptorSet> descriptor_sets = descriptor_sets_result.value();


	// Pipeline
	initium::LayoutParams layout_params = {
		.set_layouts = { set_layout },
		.push_ranges = {}
	};

	auto layout_result = initium::createPipelineLayout(device, layout_params);
	if (!layout_result.has_value()) {
		printf("Failed to create pipeline layout\n");
		return 1;
	}
	VkPipelineLayout layout = layout_result.value();

	initium::ViewportParams viewport_params = {
		.width = (float)surface_width,
		.height = (float)surface_height
	};

	initium::ScissorParams scissor_params = {
		.width = (unsigned int)surface_width,
		.height = (unsigned int)surface_height
	};

	initium::PipelineParams pipeline_params = {
		.shaders = {vert_define, frag_define},

		.vertex_binding_descriptors = { binding_descriptors },
		.vertex_attribute_descriptors = attribute_descriptors,

		.viewports = { viewport_params },
		.scissors = { scissor_params },

		.cull_mode = VK_CULL_MODE_BACK_BIT,
		.front_face = VK_FRONT_FACE_COUNTER_CLOCKWISE,

		.attachment_params = { fb_attachment }
	};
	auto pipeline_result = initium::createPipeline(device, layout, pass, pipeline_params);
	if (!pipeline_result.has_value()) {
		printf("Failed to create pipeline\n");
		return 1;
	}
	VkPipeline pipeline = pipeline_result.value();

	// Framebuffers
	initium::FramebufferParams framebuffer_params = {
		.image_views = swapchain_views,
		.render_pass = pass,
		.width = surface_width,
		.height = surface_height
	};

	auto framebuffers_result = initium::createFramebufferFromViews(device, framebuffer_params);
	if (!framebuffers_result.has_value()) {
		printf("Failed to create framebuffer(s)");
		return 1;
	}
	std::vector<VkFramebuffer> framebuffers = framebuffers_result.value();

	// Command submission
	initium::PoolParams pool_params = {
		.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		.queue_family_indices = queue_requests[0].family_indice
	};

	auto command_pool_result = initium::createCommandPool(device, pool_params);
	if (!command_pool_result.has_value()) {
		printf("Failed to allocate command pool\n");
		return 1;
	}
	VkCommandPool command_pool = command_pool_result.value();

	std::vector<VkCommandBuffer> command_buffers(max_frames_in_flight);
	auto command_buffer_result = initium::allocateCommandBuffer(device, command_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, max_frames_in_flight);
	if (!command_buffer_result.has_value()) {
		printf("Failed to allocate command buffer\n");
		return 1;
	}
	command_buffers = command_buffer_result.value();

	VkClearValue clear_colour = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
	VkViewport viewport = viewport_params.toViewport();
	VkRect2D scissor = scissor_params.toRect2D();

	// Frame synchronization 
	std::vector<VkSemaphore> image_available(max_frames_in_flight);
	std::vector<VkSemaphore> render_finished(max_frames_in_flight);
	std::vector<VkFence> in_flight(max_frames_in_flight);

	VkSemaphoreCreateInfo sem_create_info{};
	sem_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	for (int i = 0; i < max_frames_in_flight; i++) {
		if ((vkCreateSemaphore(device, &sem_create_info, nullptr, &image_available[i]) != VK_SUCCESS) ||
			(vkCreateSemaphore(device, &sem_create_info, nullptr, &render_finished[i]) != VK_SUCCESS)) {
			printf("Failed to create semaphore(s)\n");
			return 1;
		}
	}

	VkFenceCreateInfo fen_create_info{};
	fen_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fen_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	for (int i = 0; i < max_frames_in_flight; i++) {
		if (vkCreateFence(device, &fen_create_info, nullptr, &in_flight[i]) != VK_SUCCESS) {
			printf("Failed to create fence\n");
			return 1;
		}
	}


	// Render loop
	uint64_t frame_number = 0;
	printf("@@@@ INITIALISATION COMPLETE @@@@\n");
	while (!glfwWindowShouldClose(window)) {
		uint8_t i_f_index = frame_number % max_frames_in_flight;

		vkWaitForFences(device, 1, &in_flight[i_f_index], VK_TRUE, UINT64_MAX);
		vkResetFences(device, 1, &in_flight[i_f_index]);

		BindingUniformObject ubo{};
		ubo.model = glm::rotate(glm::mat4(1.0f), glm::radians((float)frame_number / 128.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		ubo.proj = glm::perspective(glm::radians(45.0f), (float)surface_width / (float)surface_height, 0.1f, 10.0f);
		ubo.proj[1][1] *= -1.0f;
		ubo.mvp = ubo.proj * ubo.view * ubo.model;
		memcpy(binding_buffer_mappings[i_f_index], &ubo, sizeof(BindingUniformObject));

		VkDescriptorBufferInfo desc_buffer_info{};
		desc_buffer_info.buffer = binding_uniform_buffers[i_f_index];
		desc_buffer_info.offset = 0;
		desc_buffer_info.range = sizeof(BindingUniformObject);

		VkWriteDescriptorSet desc_write{};
		desc_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		desc_write.dstSet = descriptor_sets[i_f_index];
		desc_write.dstBinding = 0;
		desc_write.dstArrayElement = 0;
		desc_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		desc_write.descriptorCount = 1;
		desc_write.pBufferInfo = &desc_buffer_info;
		vkUpdateDescriptorSets(device, 1, &desc_write, 0, nullptr);

		uint32_t free_image_index;
		if (vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, image_available[i_f_index], VK_NULL_HANDLE, &free_image_index) != VK_SUCCESS) {
			printf("Failed to acquire next image\n");
			return 1;
		}

		vkResetCommandBuffer(command_buffers[i_f_index], 0);

		VkRenderPassBeginInfo pass_begin_info{};
		pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		pass_begin_info.renderPass = pass;
		pass_begin_info.framebuffer = framebuffers[free_image_index];
		pass_begin_info.renderArea.offset = { 0, 0 };
		pass_begin_info.renderArea.extent = VkExtent2D{ .width = (unsigned int)surface_width, .height = (unsigned int)surface_height };
		pass_begin_info.clearValueCount = 1;
		pass_begin_info.pClearValues = &clear_colour;

		if (initium::recordCommandBuffer(command_buffers[i_f_index], 0, 
			[pass_begin_info, pipeline, layout, viewport, scissor, 
			vertex_buffer, index_buffer, descriptor_sets, vertices, indices, i_f_index]
			(VkCommandBuffer command_buffer) 
			
			{
			vkCmdBeginRenderPass(command_buffer, &pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
			vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
			vkCmdSetViewport(command_buffer, 0, 1, &viewport);
			vkCmdSetScissor(command_buffer, 0, 1, &scissor);

			VkDeviceSize vertex_offsets = { 0 };
			vkCmdBindVertexBuffers(command_buffer, 0, 1, &vertex_buffer, &vertex_offsets);
			vkCmdBindIndexBuffer(command_buffer, index_buffer, 0, VK_INDEX_TYPE_UINT16);
			vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 1, &descriptor_sets[i_f_index], 0, nullptr);
			vkCmdDrawIndexed(command_buffer, indices.size(), 1, 0, 0, 0);

			vkCmdEndRenderPass(command_buffer);
		}) != VK_SUCCESS) {
			printf("Failed to record command buffer\n");
			return 1;
		}


		// Submission
		VkPipelineStageFlags wait_stages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		VkSubmitInfo submit_info{};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &command_buffers[i_f_index];
		submit_info.waitSemaphoreCount = 1;
		submit_info.pWaitSemaphores = &image_available[i_f_index];
		submit_info.signalSemaphoreCount = 1;
		submit_info.pSignalSemaphores = &render_finished[i_f_index];
		submit_info.pWaitDstStageMask = &wait_stages;

		if (vkQueueSubmit(graphics_queue, 1, &submit_info, in_flight[i_f_index]) != VK_SUCCESS) {
			printf("Failed to submit command buffer\n");
			return 1;
		}

		// Presentation
		VkPresentInfoKHR present_info{};
		present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		present_info.waitSemaphoreCount = 1;
		present_info.pWaitSemaphores = &render_finished[i_f_index];
		present_info.swapchainCount = 1;
		present_info.pSwapchains = &swapchain;
		present_info.pImageIndices = &free_image_index;
		if (vkQueuePresentKHR(graphics_queue, &present_info) != VK_SUCCESS) {
			printf("Failed to present\n");
			return 1;
		}

		glfwPollEvents();

		frame_number++;
	}

	vkDeviceWaitIdle(device);

	for (size_t i = 0; i < max_frames_in_flight; i++) {
        vkDestroyBuffer(device, binding_uniform_buffers[i], nullptr);
        vkFreeMemory(device, binding_uniform_memory[i], nullptr);
    }

	vkDestroyDescriptorPool(device, descriptor_pool, nullptr);
	vkDestroyDescriptorSetLayout(device, set_layout, nullptr);

	vkDestroyBuffer(device, vertex_buffer, nullptr);
	vkFreeMemory(device, vertex_memory, nullptr);
	vkDestroyBuffer(device, index_buffer, nullptr);
	vkFreeMemory(device, index_memory, nullptr);

	// Cleanup
	for (int i = 0; i < max_frames_in_flight; i++) {
		vkDestroyFence(device, in_flight[i], nullptr);
		vkDestroySemaphore(device, render_finished[i], nullptr);
		vkDestroySemaphore(device, image_available[i], nullptr);
	}

	vkFreeCommandBuffers(device, command_pool, max_frames_in_flight, command_buffers.data());
	vkDestroyCommandPool(device, command_pool, nullptr);

	for (auto framebuffer : framebuffers) {
		vkDestroyFramebuffer(device, framebuffer, nullptr);
	}

	vkDestroyPipeline(device, pipeline, nullptr);
	vkDestroyPipelineLayout(device, layout, nullptr);
	vkDestroyRenderPass(device, pass, nullptr);
	vkDestroyShaderModule(device, vert_shader, nullptr);
	vkDestroyShaderModule(device, frag_shader, nullptr);

	for (auto view : swapchain_views) {
		vkDestroyImageView(device, view, nullptr);
	}

	vkDestroySwapchainKHR(device, swapchain, nullptr);
	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyDevice(device, nullptr);

	destroyDebugUtilsMessengerEXT(instance, debug_messenger, nullptr);
	vkDestroyInstance(instance, nullptr);

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}