#pragma once

#include "lve_device.h"

typedef struct lve_pipeline_config_info{
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

lve_pipeline_config_info* lvepili_default_pipeline_config_info(
    uint32_t width, uint32_t height);