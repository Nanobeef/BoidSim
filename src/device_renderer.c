#include "device_renderer.h"

#include "embed.h"


DeviceRendererLimits device_renderer_limits = {0};

DeviceRenderer create_device_renderer(Device *device, DeviceRendererCreateInfo *pointer_create_info, u32 frame_count, uvec2 frame_size, Arena* resize_arena, Arena *arena)
{


	DeviceRenderer r = {
		.device = device,
		.frame_count = frame_count,
		.frame_size = frame_size,
		.target_format = VK_FORMAT_B8G8R8A8_UNORM,
		.target_sample_count = VK_SAMPLE_COUNT_1_BIT,
		.target_image_usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
	};

	if(device_renderer_limits.initialized == 0)
	{
		device_renderer_limits = (DeviceRendererLimits){.initialized = true};
		VkImageFormatProperties format_properties = {0};
		vkGetPhysicalDeviceImageFormatProperties(r.device->physical_device, r.target_format, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL, r.target_image_usage, 0, &format_properties);
		device_renderer_limits.sample_counts = format_properties.sampleCounts;
	}

	DeviceRendererCreateInfo create_info = {0};
	if(pointer_create_info)
	{
		create_info = *pointer_create_info;
	}
	if(create_info.desired_sample_count & device_renderer_limits.sample_counts)
	{
		r.target_sample_count = create_info.desired_sample_count & device_renderer_limits.sample_counts;
	}
	else
	{
		r.target_sample_count = device_renderer_limits.sample_counts;
	}
	r.target_sample_count = uint_max_bit_set(r.target_sample_count);

	{
		VkSamplerCreateInfo info = {
			.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
			.magFilter = VK_FILTER_LINEAR,
			.minFilter = VK_FILTER_LINEAR,
			.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST,
			.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
			.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
			.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
			.mipLodBias = 1.0f,
			.anisotropyEnable = VK_FALSE,
			.maxAnisotropy = 1.0f,
			.compareEnable = VK_FALSE,
			.compareOp = VK_COMPARE_OP_LESS,
			.minLod = 1.0f,
			.maxLod = 1.0f,
			.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
			.unnormalizedCoordinates = VK_FALSE,
		};
		VK_CALL(vkCreateSampler(device->handle, &info, vkb, &r.vertex2_color_sampler), true);
	}

	{
		VkSamplerCreateInfo info = {
			.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
			.magFilter = VK_FILTER_LINEAR,
			.minFilter = VK_FILTER_LINEAR,
			.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST,
			.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
			.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
			.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
			.mipLodBias = 1.0f,
			.anisotropyEnable = VK_FALSE,
			.maxAnisotropy = 1.0f,
			.compareEnable = VK_FALSE,
			.compareOp = VK_COMPARE_OP_LESS,
			.minLod = 1.0f,
			.maxLod = 1.0f,
			.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
			.unnormalizedCoordinates = VK_FALSE,
		};
		VK_CALL(vkCreateSampler(device->handle, &info, vkb, &r.vertex2_mono_sampler), true);
	}

	{
		VkDescriptorPoolSize sizes[] = {
			{
				.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 
				.descriptorCount = frame_count * 2,
			},
		};
		VkDescriptorPoolCreateInfo info = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.pPoolSizes = sizes,
			.poolSizeCount = arrlen(sizes),
			.maxSets = frame_count,
		};
		VK_CALL(vkCreateDescriptorPool(device->handle, &info, vkb, &r.descriptor_pool), true);
	}

	{
		VkDescriptorSetLayoutBinding bindings[] = {
			{
				.binding = 0,
				.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				.descriptorCount = 1,
				.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
				.pImmutableSamplers = &r.vertex2_color_sampler,
			},
			{
				.binding = 1,
				.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				.descriptorCount = 1,
				.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
				.pImmutableSamplers = &r.vertex2_mono_sampler,
			}
		};
		VkDescriptorSetLayoutCreateInfo info = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.pBindings = bindings,
			.bindingCount = arrlen(bindings),
		};
		VK_CALL(vkCreateDescriptorSetLayout(device->handle, &info, vkb, &r.descriptor_set_layout), true);
	}
	
	{
		VkPushConstantRange range = {
			.size = sizeof(DeviceRendererPushRange),
			.offset = 0,
			.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
		};

		VkPipelineLayoutCreateInfo info = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			.pushConstantRangeCount = 1,
			.pPushConstantRanges = &range,
			.setLayoutCount = 1,
			.pSetLayouts = &r.descriptor_set_layout,
		};
		VK_CALL(vkCreatePipelineLayout(device->handle, &info, vkb, &r.pipeline_layout), true);
	}

	if(r.target_sample_count == VK_SAMPLE_COUNT_1_BIT){
		VkAttachmentReference color_reference = {
			.attachment = 0,
			.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		};

		VkAttachmentDescription attachments[] = {
			{
				.format = r.target_format,
				.samples = VK_SAMPLE_COUNT_1_BIT,
				.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
				.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
				.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
				.finalLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			},
		};

		VkSubpassDescription subpasses[] = {
			{
				.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
				.colorAttachmentCount = 1,
				.pColorAttachments = &color_reference,
			},
		};

		VkSubpassDependency dependencies[] = {
			{
				.srcSubpass = 0,
				.dstSubpass = VK_SUBPASS_EXTERNAL,
				.srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
				.dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT,
				.srcAccessMask = VK_ACCESS_NONE,
				.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
				.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
			},
		};

		VkRenderPassCreateInfo info = {
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
			.attachmentCount = arrlen(attachments),
			.pAttachments = attachments,
			.subpassCount = arrlen(subpasses),
			.pSubpasses = subpasses,
			.dependencyCount = arrlen(dependencies),
			.pDependencies = dependencies,

		};

		r.render_pass = create_render_pass(device, info, arena);
	}
	else
	{
		VkAttachmentReference color_reference = {
			.attachment = 0,
			.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		};
		VkAttachmentReference resolve_reference = {
			.attachment = 1,
			.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		};

		VkAttachmentDescription attachments[] = {
			{
				.format = r.target_format,
				.samples = r.target_sample_count,
				.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
				.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
				.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			},
			{
				.format = r.target_format,
				.samples = VK_SAMPLE_COUNT_1_BIT,
				.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
				.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
				.finalLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			},
		};

		VkSubpassDescription subpasses[] = {
			{
				.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
				.colorAttachmentCount = 1,
				.pColorAttachments = &color_reference,
				.pResolveAttachments = &resolve_reference,
			},
		};

		VkSubpassDependency dependencies[] = {
			{
				.srcSubpass = 0,
				.dstSubpass = VK_SUBPASS_EXTERNAL,
				.srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
				.dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT,
				.srcAccessMask = VK_ACCESS_NONE,
				.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
				.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
			},
		};

		VkRenderPassCreateInfo info = {
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
			.attachmentCount = arrlen(attachments),
			.pAttachments = attachments,
			.subpassCount = arrlen(subpasses),
			.pSubpasses = subpasses,
			.dependencyCount = arrlen(dependencies),
			.pDependencies = dependencies,

		};

		r.render_pass = create_render_pass(device, info, arena);
	}

	{
		ShaderModule vertex2_vert_module = create_shader_module(device,
			(u64)(_binary_bin_vertex2_vert_spv_end - _binary_bin_vertex2_vert_spv_start),	
			(u32*)_binary_bin_vertex2_vert_spv_start
		);
		ShaderModule vertex2_frag_module = create_shader_module(device,
			(u64)(_binary_bin_vertex2_frag_spv_end - _binary_bin_vertex2_frag_spv_start),	
			(u32*)_binary_bin_vertex2_frag_spv_start
		);
		ShaderModule boid_vert_module = create_shader_module(device,
			(u64)(_binary_bin_boid_vert_spv_end - _binary_bin_boid_vert_spv_start),	
			(u32*)_binary_bin_boid_vert_spv_start
		);
		ShaderModule boid_frag_module = create_shader_module(device,
			(u64)(_binary_bin_boid_frag_spv_end - _binary_bin_boid_frag_spv_start),	
			(u32*)_binary_bin_boid_frag_spv_start
		);



		VkPipelineShaderStageCreateInfo vertex2_vertex_stage = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,	
			.stage = VK_SHADER_STAGE_VERTEX_BIT,
			.module = vertex2_vert_module.handle,
			.pName = "main",
		};

		VkPipelineShaderStageCreateInfo vertex2_fragment_stage = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,	
			.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
			.module = vertex2_frag_module.handle,
			.pName = "main",
		};

		VkPipelineShaderStageCreateInfo vertex2_stages[] = {
			vertex2_vertex_stage,
			vertex2_fragment_stage,
		};

		VkVertexInputBindingDescription vertex2_bindings[] = {
			{
				.binding = 0,
				.stride = sizeof(Vertex2),
				.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
			},
		};

		VkVertexInputAttributeDescription vertex2_attributes[] = {
			{
				.binding = 0,
				.location = 0,
				.format = VK_FORMAT_R32G32B32A32_SFLOAT,
				.offset = offsetof(Vertex2, color),
			},
			{
				.binding = 0,
				.location = 1,
				.format = VK_FORMAT_R32G32_SFLOAT,
				.offset = offsetof(Vertex2, position),
			},
			{
				.binding = 0,
				.location = 2,
				.format = VK_FORMAT_R32_UINT,
				.offset = offsetof(Vertex2, texture),
			},
		};

		VkPipelineVertexInputStateCreateInfo vertex2_input_state = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,	
			.vertexBindingDescriptionCount = arrlen(vertex2_bindings),
			.pVertexBindingDescriptions = vertex2_bindings,
			.vertexAttributeDescriptionCount = arrlen(vertex2_attributes),
			.pVertexAttributeDescriptions = vertex2_attributes,
		};

		VkPipelineShaderStageCreateInfo boid_vertex_stage = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,	
			.stage = VK_SHADER_STAGE_VERTEX_BIT,
			.module = boid_vert_module.handle,
			.pName = "main",
		};

		VkPipelineShaderStageCreateInfo boid_fragment_stage = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,	
			.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
			.module = boid_frag_module.handle,
			.pName = "main",
		};


		VkPipelineShaderStageCreateInfo boid_stages[] = {
			boid_vertex_stage,
			boid_fragment_stage,
		};

		VkVertexInputBindingDescription boid_bindings[] = {
			{
				.binding = 0,
				.stride = sizeof(fvec2),
				.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE,
			},
			{
				.binding = 1,
				.stride = sizeof(fvec2),
				.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE,
			},
		};

		VkVertexInputAttributeDescription boid_attributes[] = {
			{
				.binding = 0,
				.location = 0,
				.format = VK_FORMAT_R32G32_SFLOAT,
				.offset = 0,
			},
			{
				.binding = 1,
				.location = 1,
				.format = VK_FORMAT_R32G32_SFLOAT,
				.offset = 0,
			},
		};

		VkPipelineVertexInputStateCreateInfo boid_input_state = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,	
			.vertexBindingDescriptionCount = arrlen(boid_bindings),
			.pVertexBindingDescriptions = boid_bindings,
			.vertexAttributeDescriptionCount = arrlen(boid_attributes),
			.pVertexAttributeDescriptions = boid_attributes,
		};
		// Boids End

		VkPipelineInputAssemblyStateCreateInfo point_list_input_assembly_state = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,	
			.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST,
			.primitiveRestartEnable = VK_FALSE,
		};

		VkPipelineInputAssemblyStateCreateInfo line_list_input_assembly_state = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,	
			.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
			.primitiveRestartEnable = VK_FALSE,
		};

		VkPipelineInputAssemblyStateCreateInfo triangle_list_input_assembly_state = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,	
			.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
			.primitiveRestartEnable = VK_FALSE,
		};

		VkPipelineInputAssemblyStateCreateInfo triangle_strip_input_assembly_state = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,	
			.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
			.primitiveRestartEnable = VK_FALSE,
		};

		VkPipelineInputAssemblyStateCreateInfo line_strip_input_assembly_state = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,	
			.topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP,
			.primitiveRestartEnable = VK_FALSE,
		};

		VkPipelineTessellationStateCreateInfo tessellation_state = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO,
			.patchControlPoints = 32,
		};

		VkViewport viewport = {
			.x = 0.00,	
			.y = 0.0,
			.width = 1000.0,
			.height = 1000.0,
			.minDepth = 0.0,
			.maxDepth = 1.0,
		};

		VkRect2D scissor = {
			.offset = (VkOffset2D){0,0},
			.extent = (VkExtent2D){0,0},
		};

		VkPipelineViewportStateCreateInfo viewport_state = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
			.viewportCount = 1,
			.pViewports = &viewport,
			.scissorCount = 1,
			.pScissors = &scissor,
		};

		VkPipelineRasterizationStateCreateInfo rasterization_state = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
			.depthClampEnable = VK_FALSE,
			.rasterizerDiscardEnable = VK_FALSE,
			.polygonMode = VK_POLYGON_MODE_FILL,
			.frontFace = VK_FRONT_FACE_CLOCKWISE,
			.depthBiasEnable = VK_FALSE,
			.depthBiasConstantFactor = 1.0f,
			.depthBiasClamp = 0.0f,
			.depthBiasSlopeFactor = 1.0f,
			.lineWidth = 1.0f,
		};

		VkPipelineRasterizationStateCreateInfo line_rasterization_state = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
			.depthClampEnable = VK_FALSE,
			.rasterizerDiscardEnable = VK_FALSE,
			.polygonMode = VK_POLYGON_MODE_LINE,
			.frontFace = VK_FRONT_FACE_CLOCKWISE,
			.depthBiasEnable = VK_FALSE,
			.depthBiasConstantFactor = 1.0f,
			.depthBiasClamp = 0.0f,
			.depthBiasSlopeFactor = 1.0f,
			.lineWidth = 1.0f,
		};

		VkSampleMask sample_mask = 0xFFFFFFFF;

		VkPipelineMultisampleStateCreateInfo multisample_state = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
			.rasterizationSamples = r.target_sample_count,
			.sampleShadingEnable = VK_FALSE,
			.minSampleShading = 0.0f,
			.pSampleMask = NULL,
			.alphaToCoverageEnable = VK_FALSE,
			.alphaToOneEnable = VK_FALSE,
		};

		VkPipelineDepthStencilStateCreateInfo depth_stencil_state = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
			.depthTestEnable = VK_FALSE,
			.depthWriteEnable = VK_FALSE,
			.depthCompareOp = VK_COMPARE_OP_LESS,
			.depthBoundsTestEnable = VK_FALSE,
			.stencilTestEnable = VK_FALSE,
			.front = (VkStencilOpState){0},
			.back = (VkStencilOpState){0},
			.minDepthBounds = 0.0f,
			.maxDepthBounds = 1.0f,
		};
		
		VkPipelineColorBlendAttachmentState color_blend_attachments[] = {
			{
				.blendEnable = VK_TRUE,
				.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
				.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
				.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
				.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,

				.colorBlendOp = VK_BLEND_OP_ADD,
				.alphaBlendOp = VK_BLEND_OP_ADD,
				.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,  
			},
		};

		VkPipelineColorBlendStateCreateInfo color_blend_state = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
			.logicOpEnable = VK_FALSE,
			.logicOp = VK_LOGIC_OP_SET,
			.attachmentCount = arrlen(color_blend_attachments),
			.pAttachments = color_blend_attachments,
			.blendConstants[0] = 0.0f,
			.blendConstants[1] = 0.0f,
			.blendConstants[2] = 0.0f,
			.blendConstants[3] = 0.0f,
		};

		VkPipelineColorBlendAttachmentState no_blend_attachments[] = {
			{
				.blendEnable = VK_FALSE,
				.srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
				.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
				.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
				.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,

				.colorBlendOp = VK_BLEND_OP_ADD,
				.alphaBlendOp = VK_BLEND_OP_ADD,
				.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,  
			},
		};

		VkPipelineColorBlendStateCreateInfo no_color_blend_state = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
			.logicOpEnable = VK_FALSE,
			.logicOp = VK_LOGIC_OP_SET,
			.attachmentCount = arrlen(no_blend_attachments),
			.pAttachments = no_blend_attachments,
			.blendConstants[0] = 0.0f,
			.blendConstants[1] = 0.0f,
			.blendConstants[2] = 0.0f,
			.blendConstants[3] = 0.0f,
		};
		
		VkDynamicState dynamic_states[] = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR,
		};

		VkPipelineDynamicStateCreateInfo dynamic_state = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
			.dynamicStateCount = arrlen(dynamic_states),
			.pDynamicStates = dynamic_states,
		};

		VkGraphicsPipelineCreateInfo base_info = {
			.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
			.stageCount = arrlen(vertex2_stages),
			.pStages = vertex2_stages,
			.pVertexInputState = &vertex2_input_state,
			.pInputAssemblyState = &triangle_list_input_assembly_state,
			.pTessellationState = &tessellation_state,
			.pViewportState = &viewport_state,
			.pRasterizationState = &rasterization_state,
			.pMultisampleState = &multisample_state,
			.pDepthStencilState = &depth_stencil_state,
			.pColorBlendState = &color_blend_state,
			.pDynamicState = &dynamic_state,
			.renderPass = r.render_pass.handle,
			.subpass = 0,
			.layout = r.pipeline_layout,
		};
		
		VkGraphicsPipelineCreateInfo infos[4];

		infos[0] = base_info;
		infos[1] = base_info;
		infos[1].pRasterizationState = &line_rasterization_state;
		base_info.pStages = boid_stages;
		base_info.pVertexInputState = &boid_input_state;
		base_info.pColorBlendState = &no_color_blend_state;
		infos[2] = base_info;
		infos[3] = base_info;
		infos[3].pRasterizationState = &line_rasterization_state;

		VkPipeline pipelines[arrlen(infos)];
		
		VK_CALL(vkCreateGraphicsPipelines(device->handle, 0, arrlen(infos), infos, vkb, pipelines), true);

		r.vertex2_pipeline = pipelines[0];
		r.vertex2_wireframe_pipeline = pipelines[1];
		r.boid_pipeline = pipelines[2];
		r.boid_wireframe_pipeline = pipelines[3];


		destroy_shader_module(boid_vert_module);
		destroy_shader_module(boid_frag_module);
		destroy_shader_module(vertex2_vert_module);
		destroy_shader_module(vertex2_frag_module);
	}

// ========================================================================================
// SHARED DATA
// ========================================================================================

	r.texture_image = allocate_device_image2d(device->explicit_arena, uvec2_make(2048, 2048), r.target_format, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, arena);
	r.texture_image_view = create_device_image_view2d(r.texture_image);

	r.glyph_image = allocate_device_image2d(device->explicit_arena, uvec2_make(2048, 2048), VK_FORMAT_R8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, arena);
	r.glyph_image_view = create_device_image_view2d(r.glyph_image);


	r.simple_font = create_simple_font(device->explicit_arena, r.glyph_image, 32, arena);
	r.world_camera = make_camera2(frame_size);
	r.overlay_camera = make_camera2(frame_size);
	r.blit_camera = make_camera2(frame_size);

	r.descriptor_sets = arena_alloc(sizeof(VkDescriptorSet) * frame_count,1,0, arena);

	r.ui_vertex_buffers = arena_alloc(sizeof(DeviceVertexBuffer) * frame_count,1,0, arena);
	r.overlay_vertex_buffers = arena_alloc(sizeof(DeviceVertexBuffer) * frame_count,1,0, arena);
	r.world_vertex_buffers = arena_alloc(sizeof(DeviceVertexBuffer) * frame_count,1,0, arena);

	for(u32 i = 0; i < frame_count; i++)
	{
		r.overlay_vertex_buffers[i] = create_vertex_buffer(device->explicit_arena, sizeof(Vertex2), U16_MAX * sizeof(Vertex2), arena);
		r.ui_vertex_buffers[i] = create_vertex_buffer(device->explicit_arena, sizeof(Vertex2), U16_MAX * sizeof(Vertex2), arena);
		r.world_vertex_buffers[i] = create_vertex_buffer(device->explicit_arena, sizeof(Vertex2), U16_MAX * sizeof(Vertex2), arena);
	}
	
	DeviceImageView2D descriptor_image_views[] = {
		r.texture_image_view,
		r.glyph_image_view,
	};

	VkWriteDescriptorSet writes[arrlen(descriptor_image_views)];
	VkDescriptorImageInfo image_infos[arrlen(descriptor_image_views)];

	for(u32 i = 0; i < frame_count; i++)
	{
		VkDescriptorSetAllocateInfo info = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.descriptorPool = r.descriptor_pool,
			.descriptorSetCount = 1,
			.pSetLayouts = &r.descriptor_set_layout,
		};
		VK_CALL(vkAllocateDescriptorSets(device->handle, &info, &r.descriptor_sets[i]), true);


		for(u32 j = 0; j < arrlen(descriptor_image_views); j++){
			image_infos[j] = (VkDescriptorImageInfo) {
				.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,			
				.imageView = descriptor_image_views[j].handle,
			};
			writes[j] = (VkWriteDescriptorSet){
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,				
				.dstSet = r.descriptor_sets[i],
				.dstBinding = j,
				.descriptorCount = 1,
				.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				.pImageInfo = &image_infos[j],
			};
		}
		vkUpdateDescriptorSets(device->handle, arrlen(writes), writes, 0,0);
	}

	
	{
		TransientDeviceCommandBuffer tcb = begin_transient_device_command_buffer(device->main_queue_family->queues[0]);
		cmd_transition_device_image2d(tcb.cb, r.texture_image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_NONE, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
		cmd_transition_device_image2d(tcb.cb, r.glyph_image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_NONE, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
		end_transient_device_command_buffer(tcb);
		wait_transient_device_command_buffer(tcb);
	}



// ========================================================================================
// FRAME DATA
// ========================================================================================

	r.target_images = arena_alloc(sizeof(DeviceImage2D) * frame_count,1,0, arena);
	r.target_image_views = arena_alloc(sizeof(DeviceImageView2D) * frame_count,1,0, arena);
	r.target_msaa_images = arena_alloc(sizeof(DeviceImage2D) * frame_count,1,0, arena);
	r.target_msaa_image_views = arena_alloc(sizeof(DeviceImageView2D) * frame_count, 1,0,arena);

	r.framebuffers = arena_alloc(sizeof(Framebuffer) * frame_count,1,0, arena);

	resize_renderer_create(&r, r.frame_size, resize_arena);

	return r;
}

void resize_renderer_create(DeviceRenderer *r, uvec2 frame_size,Arena *resize_arena)
{
	r->frame_size = frame_size;
	Device *device = r->device;
	for(u32 i = 0; i < r->frame_count; i++)
	{
		r->target_images[i] = allocate_device_image2d(device->explicit_arena, r->frame_size, r->target_format, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, resize_arena);
		r->target_image_views[i] = create_device_image_view2d(r->target_images[i]);

		if(r->target_sample_count == VK_SAMPLE_COUNT_1_BIT)
		{
			DeviceImageView2D attachments[] = {r->target_image_views[i]};
			r->framebuffers[i] = create_framebuffer(r->render_pass, r->frame_size, arrlen(attachments), attachments);
		}
		else
		{
			r->target_msaa_images[i] = allocate_device_image2d_long(device->explicit_arena, r->frame_size, r->target_format, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, r->target_sample_count, VK_IMAGE_TILING_OPTIMAL,resize_arena);
			r->target_msaa_image_views[i] = create_device_image_view2d(r->target_msaa_images[i]);
			DeviceImageView2D attachments[] = {r->target_msaa_image_views[i], r->target_image_views[i]};
			r->framebuffers[i] = create_framebuffer(r->render_pass, r->frame_size, arrlen(attachments), attachments);
		}
	}
	r->world_camera = resize_camera2(r->world_camera, r->frame_size);
	r->overlay_camera = resize_camera2(r->overlay_camera, r->frame_size);
	r->blit_camera = resize_camera2(r->blit_camera, r->frame_size);
}

void resize_renderer_destroy(DeviceRenderer *r)
{
	Device *device = r->device;
	for(u32 i = 0; i < r->frame_count; i++)
	{
		destroy_framebuffer(r->framebuffers[i]);
		destroy_device_image_view2d(r->target_image_views[i]);
		free_device_image2d(r->target_images[i]);
		if(r->target_sample_count != VK_SAMPLE_COUNT_1_BIT)
		{
			free_device_image2d(r->target_msaa_images[i]);
			destroy_device_image_view2d(r->target_msaa_image_views[i]);	
		}
	}
}

void destroy_device_renderer(DeviceRenderer r)
{

	resize_renderer_destroy(&r);

	for(u32 i = 0; i < r.frame_count; i++)
	{
		destroy_vertex_buffer(r.overlay_vertex_buffers[i]);
		destroy_vertex_buffer(r.ui_vertex_buffers[i]);
		destroy_vertex_buffer(r.world_vertex_buffers[i]);
	}

	destroy_device_image_view2d(r.texture_image_view);
	free_device_image2d(r.texture_image);
	destroy_device_image_view2d(r.glyph_image_view);
	free_device_image2d(r.glyph_image);

	vkDestroyPipeline(r.device->handle, r.vertex2_pipeline, vkb);
	vkDestroyPipeline(r.device->handle, r.vertex2_wireframe_pipeline, vkb);
	vkDestroyPipeline(r.device->handle, r.boid_pipeline, vkb);
	vkDestroyPipeline(r.device->handle, r.boid_wireframe_pipeline, vkb);

	destroy_render_pass(r.render_pass);
	vkDestroyPipelineLayout(r.device->handle, r.pipeline_layout, vkb);
	vkDestroyDescriptorSetLayout(r.device->handle, r.descriptor_set_layout, vkb);
	vkDestroyDescriptorPool(r.device->handle, r.descriptor_pool, vkb);
	vkDestroySampler(r.device->handle, r.vertex2_color_sampler, vkb);
	vkDestroySampler(r.device->handle, r.vertex2_mono_sampler, vkb);
}

void cmd_device_renderer_blit(DeviceCommandBuffer cb, DeviceRenderer dr, u32 frame_index, DeviceImage2D dst)
{
	if(dr.blit_camera.zoom < 1.0)
	{
		dr.blit_camera = make_camera2(dst.size);
	}

	// Zoom in on pixels
	// Needs a bit of work.
	fvec2 half_size = fvec2_scalar_div(fvec2_cast_uvec2(dr.frame_size), 2.0);
	fvec2 fa = fvec2_mul(dr.blit_camera.top_left, half_size);
	fvec2 fb = fvec2_mul(dr.blit_camera.bottom_right, half_size);
	svec2 a = svec2_cast_fvec2(fvec2_add(fa, half_size));
	svec2 b = svec2_cast_fvec2(fvec2_add(fb, half_size));
	if(a.x < 0)
	{
		a.x += abs(a.x);
		b.x += abs(a.x);
	}
	if(a.y < 0)
	{
		a.y += abs(a.y);
		b.y += abs(a.y);
	}
	if(b.x > dr.frame_size.x)
	{
		a.x -= dr.frame_size.x - b.x;
		b.x -= dr.frame_size.x - b.x;
	}
	if(b.y > dr.frame_size.y)
	{
		a.y -= dr.frame_size.y - b.y;
		b.y -= dr.frame_size.y - b.y;
	}

	cmd_blit_device_image2d_src_range(cb, dr.target_images[frame_index], dst, a,b);

}

void recreate_device_renderer(DeviceRenderer *dr, u32 frame_count, uvec2 frame_size, Arena *resize_arena)
{
	resize_renderer_destroy(dr);
	resize_renderer_create(dr, frame_size, resize_arena);
	{
		u32 debug_index = dr->debug_index;
		b32 show_wireframe = dr->show_wireframe;
		Camera2 world_camera = dr->world_camera;
		Camera2 blit_camera = dr->blit_camera;
		destroy_device_renderer(*dr);
		*dr = create_device_renderer(dr->device, &dr->create_info, frame_count, frame_size, resize_arena, resize_arena);
		dr->world_camera = world_camera;
		dr->blit_camera = blit_camera;
		dr->debug_index = debug_index;
		dr->show_wireframe = show_wireframe;
	}
}

b32 poll_device_renderer(DeviceRenderer *dr, FrameEvents fe, b32 move_world_camera)
{

// Update camera
	if(fe.m.pressed)
	{
		dr->blit_camera = update_camera2(dr->blit_camera,fe,true);
		if(dr->blit_camera.zoom < 1.0)
		{
			dr->blit_camera = make_camera2(dr->frame_size);
		}
		dr->blit_camera_active = true;
	}
	else
	{
		if(dr->blit_camera_active)
		{
			dr->blit_camera = make_camera2(dr->frame_size);
			dr->blit_camera_active = false;
		}
		dr->world_camera = update_camera2(dr->world_camera,fe,move_world_camera);
	}
	dr->overlay_camera = update_camera2(dr->overlay_camera,fe,false);

// Check input
	b32 force_resize = false;
	dr->create_info = (DeviceRendererCreateInfo){dr->target_sample_count};
	if(fe.n1.pressed && fe.p.pressed)
	{
		dr->create_info.desired_sample_count = VK_SAMPLE_COUNT_1_BIT;
	}
	if(fe.n2.pressed && fe.p.pressed)
	{
		dr->create_info.desired_sample_count = VK_SAMPLE_COUNT_2_BIT;
	}
	if(fe.n3.pressed && fe.p.pressed)
	{
		dr->create_info.desired_sample_count = VK_SAMPLE_COUNT_4_BIT;
	}
	if(fe.n4.pressed && fe.p.pressed)
	{
		dr->create_info.desired_sample_count = VK_SAMPLE_COUNT_8_BIT;
	}
	if(fe.n5.pressed && fe.p.pressed)
	{
		dr->create_info.desired_sample_count = VK_SAMPLE_COUNT_16_BIT;
	}
	if(fe.n0.pressed && fe.p.pressed)
	{
		dr->create_info.desired_sample_count = uint_max_bit_set(device_renderer_limits.sample_counts);
	}
	if((dr->create_info.desired_sample_count != dr->target_sample_count) && (dr->create_info.desired_sample_count & device_renderer_limits.sample_counts))
	{
		force_resize = true;
	}

	if((fe.n1.pressed || fe.n0.pressed) && fe.o.pressed)
	{
		dr->debug_index = 0;
	}
	if(fe.n2.pressed && fe.o.pressed)
	{
		dr->debug_index = 1;
	}
	if(fe.n3.pressed && fe.o.pressed)
	{
		dr->debug_index = 2;
	}
	static u64 wireframe_time = 0;
	if(fe.i.action_time != wireframe_time && fe.i.pressed)
	{
		dr->show_wireframe = !dr->show_wireframe;
		wireframe_time = fe.i.action_time;
	}
	return force_resize;
}

