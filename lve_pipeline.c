#include "lve_pipeline.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>

typedef struct lvepili_shader_buf{
    int size;
    uint8_t buf[];
} lvepili_shader_buf;

// IMPORTANT: the return buffer for this HAS to be freed
lvepili_shader_buf* lvepili_read_file(const char* file_path){
    FILE *fp = fopen(file_path, "rb");
    if (fp == NULL) {
        printf("[ERROR] Could not read file '%s'. ferror() returned the following: (%0x)\n", file_path, ferror(fp));
        return NULL;
    }
    fseek(fp, 0L, SEEK_END);
    int sz = ftell(fp);
    rewind(fp);
    lvepili_shader_buf* r = malloc(sizeof(int) + sz);
    r->size = sz;
    fgets(r->buf, sz, fp);
    fclose(fp);
    return r;
}

void lvepili_create_shader_module(lve_pipeline* lvepili, const lvepili_shader_buf* code, VkShaderModule* shader_module){
    VkShaderModuleCreateInfo create_info;
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = code->size; // could need a +1
    create_info.pCode = (const uint32_t*)code->buf; // there's an incredibly high chance this doesnt work
    if (vkCreateShaderModule(lvedev_device(lvepili->m_device), &create_info, NULL, shader_module) != VK_SUCCESS)
        printf("[ERROR] lvepili_create_shader_module() failed!\n");
}

void lvepili_create_graphics_pipeline(
    lve_pipeline* lvepili,
    const char* vert_file_path,
    const char* frag_file_path,
    const lve_pipeline_config_info* config_info)
{
    assert(config_info->pipeline_layout != VK_NULL_HANDLE &&
        "[ERROR] Cannot create graphics pipeline! no pipeline_layout provided in config_info\n");
    assert(config_info->render_pass != VK_NULL_HANDLE && 
        "[ERROR] Cannot create graphics pipeline! no render_pass provided in config_info\n");

    lvepili_shader_buf* vert_code = lvepili_read_file(vert_file_path);
    lvepili_shader_buf* frag_code = lvepili_read_file(frag_file_path);
    printf("[INFO] Vert shader size: %d\n", vert_code->size);
    printf("[INFO] Frag shader size: %d\n", frag_code->size);

    lvepili_create_shader_module(lvepili, vert_code, &lvepili->m_vert_shader_module);
    lvepili_create_shader_module(lvepili, frag_code, &lvepili->m_frag_shader_module);

    VkPipelineShaderStageCreateInfo shader_stages[2];

    shader_stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader_stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shader_stages[0].module = lvepili->m_vert_shader_module;
    shader_stages[0].pName = "main";
    shader_stages[0].flags = 0;
    shader_stages[0].pNext = NULL;
    shader_stages[0].pSpecializationInfo = NULL;

    shader_stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader_stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shader_stages[1].module = lvepili->m_frag_shader_module;
    shader_stages[1].pName = "main";
    shader_stages[1].flags = 0;
    shader_stages[1].pNext = NULL;
    shader_stages[1].pSpecializationInfo = NULL;

    VkPipelineVertexInputStateCreateInfo vertex_input_info;
    vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_info.vertexAttributeDescriptionCount = 0;
    vertex_input_info.vertexBindingDescriptionCount = 0;
    vertex_input_info.pVertexAttributeDescriptions = NULL;
    vertex_input_info.pVertexBindingDescriptions = NULL;

    VkGraphicsPipelineCreateInfo pipeline_info;
    pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_info.stageCount = 2;
    pipeline_info.pStages = shader_stages;
    pipeline_info.pVertexInputState = &vertex_input_info;
    pipeline_info.pInputAssemblyState = &config_info->input_assembly_info;
    pipeline_info.pViewportState = &config_info->viewport_info;
    pipeline_info.pMultisampleState = &config_info->multisample_info;
    pipeline_info.pRasterizationState = &config_info->rasterization_info;
    pipeline_info.pColorBlendState = &config_info->color_blend_info;
    pipeline_info.pDepthStencilState = &config_info->depth_stencil_info;
    pipeline_info.pDynamicState = NULL;

    pipeline_info.layout = config_info->pipeline_layout;
    pipeline_info.renderPass = config_info->render_pass;
    pipeline_info.subpass = config_info->subpass;

    pipeline_info.basePipelineIndex = -1;
    pipeline_info.basePipelineHandle = VK_NULL_HANDLE;

    if(vkCreateGraphicsPipelines(
        lvedev_device(lvepili->m_device),
        VK_NULL_HANDLE,
        1,
        &pipeline_info,
        NULL,
        &lvepili->m_graphics_pipeline) != VK_SUCCESS)
        printf("[ERROR] vkCreateGraphicsPipelines() failed!(good luck fixing this lmao)\n");

    free(vert_code);
    free(frag_code);
}

lve_pipeline* lvepili_make(
    lve_device* device,
    const char* vert_file_path,
    const char* frag_file_path,
    const lve_pipeline_config_info* config_info)
{
    lve_pipeline* r = malloc(sizeof(lve_pipeline));

    r->m_device = device;

    lvepili_create_graphics_pipeline(r, vert_file_path, frag_file_path, config_info);
    
    return r;
}

void lvepili_destroy(lve_pipeline* lvepili){
    vkDestroyShaderModule(
        lvedev_device(lvepili->m_device),
        lvepili->m_vert_shader_module,
        NULL);
    vkDestroyShaderModule(
        lvedev_device(lvepili->m_device),
        lvepili->m_frag_shader_module,
        NULL);
    vkDestroyPipeline(
        lvedev_device(lvepili->m_device),
        lvepili->m_graphics_pipeline,
        NULL);
    free(lvepili);
}

lve_pipeline_config_info* lvepili_default_pipeline_config_info(
    uint32_t width, uint32_t height)
{
    lve_pipeline_config_info* r = malloc(sizeof(lve_pipeline_config_info));

    r->input_assembly_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    r->input_assembly_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    r->input_assembly_info.primitiveRestartEnable = VK_FALSE;

    r->viewport.x = 0.0f;
    r->viewport.y = 0.0f;
    r->viewport.width = (float)width;
    r->viewport.height = (float)height;
    r->viewport.minDepth = 0.0f;
    r->viewport.maxDepth = 1.0f;

    r->scissor.offset = (VkOffset2D){0, 0};
    r->scissor.extent = (VkExtent2D){width, height};
    
    r->viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    r->viewport_info.viewportCount = 1;
    r->viewport_info.pViewports = &r->viewport;
    r->viewport_info.scissorCount = 1;
    r->viewport_info.pScissors = &r->scissor;
    
    r->rasterization_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    r->rasterization_info.depthClampEnable = VK_FALSE;
    r->rasterization_info.rasterizerDiscardEnable = VK_FALSE;
    r->rasterization_info.polygonMode = VK_POLYGON_MODE_FILL;
    r->rasterization_info.lineWidth = 1.0f;
    r->rasterization_info.cullMode = VK_CULL_MODE_NONE;
    r->rasterization_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
    r->rasterization_info.depthBiasEnable = VK_FALSE;
    r->rasterization_info.depthBiasConstantFactor = 0.0f;  // Optional
    r->rasterization_info.depthBiasClamp = 0.0f;           // Optional
    r->rasterization_info.depthBiasSlopeFactor = 0.0f;     // Optional
    
    r->multisample_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    r->multisample_info.sampleShadingEnable = VK_FALSE;
    r->multisample_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    r->multisample_info.minSampleShading = 1.0f;           // Optional
    r->multisample_info.pSampleMask = NULL;             // Optional
    r->multisample_info.alphaToCoverageEnable = VK_FALSE;  // Optional
    r->multisample_info.alphaToOneEnable = VK_FALSE;       // Optional
    
    r->color_blend_attachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
        VK_COLOR_COMPONENT_A_BIT;
    r->color_blend_attachment.blendEnable = VK_FALSE;
    r->color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
    r->color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
    r->color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;              // Optional
    r->color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
    r->color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
    r->color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;              // Optional
    
    r->color_blend_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    r->color_blend_info.logicOpEnable = VK_FALSE;
    r->color_blend_info.logicOp = VK_LOGIC_OP_COPY;  // Optional
    r->color_blend_info.attachmentCount = 1;
    r->color_blend_info.pAttachments = &r->color_blend_attachment;
    r->color_blend_info.blendConstants[0] = 0.0f;  // Optional
    r->color_blend_info.blendConstants[1] = 0.0f;  // Optional
    r->color_blend_info.blendConstants[2] = 0.0f;  // Optional
    r->color_blend_info.blendConstants[3] = 0.0f;  // Optional
    
    r->depth_stencil_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    r->depth_stencil_info.depthTestEnable = VK_TRUE;
    r->depth_stencil_info.depthWriteEnable = VK_TRUE;
    r->depth_stencil_info.depthCompareOp = VK_COMPARE_OP_LESS;
    r->depth_stencil_info.depthBoundsTestEnable = VK_FALSE;
    r->depth_stencil_info.minDepthBounds = 0.0f;  // Optional
    r->depth_stencil_info.maxDepthBounds = 1.0f;  // Optional
    r->depth_stencil_info.stencilTestEnable = VK_FALSE;
    r->depth_stencil_info.front = (VkStencilOpState){};  // Optional
    r->depth_stencil_info.back = (VkStencilOpState){};   // Optional

    r->pipeline_layout = NULL;
    r->render_pass = NULL;
    r->subpass = 0;

    return r;
}

//theres a chance that this isnt working if something glitches
//out try checking here for errors