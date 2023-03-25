#pragma once

#include "lve_device.h"

typedef struct lve_pipeline_config_info{
    VkPipelineInputAssemblyStateCreateInfo input_assembly_info;

    VkPipelineViewportStateCreateInfo viewport_info;
    VkPipelineRasterizationStateCreateInfo rasterization_info;
    VkPipelineMultisampleStateCreateInfo multisample_info;
    VkPipelineColorBlendAttachmentState color_blend_attachment;
    VkPipelineColorBlendStateCreateInfo color_blend_info;
    VkPipelineDepthStencilStateCreateInfo depth_stencil_info;
    VkDynamicState dynamic_state_enables[2];
    uint32_t dynamic_state_enables_c;
    VkPipelineDynamicStateCreateInfo dynamic_state_info;
    VkPipelineLayout pipeline_layout;
    VkRenderPass render_pass;
    uint32_t subpass;
} lve_pipeline_config_info;

typedef struct lve_pipeline{
    lve_device* m_device;
    VkPipeline m_graphics_pipeline;
    VkShaderModule m_vert_shader_module;
    VkShaderModule m_frag_shader_module;
} lve_pipeline;

lve_pipeline* lvepili_make(
    lve_device* device,
    const char* vert_file_path,
    const char* frag_file_path,
    const lve_pipeline_config_info* config_info);

void lvepili_destroy(lve_pipeline* lvepili);

void lvepili_default_pipeline_config_info(lve_pipeline_config_info* config_info);

void lvepili_bind(lve_pipeline* lvepili, VkCommandBuffer command_buffer);