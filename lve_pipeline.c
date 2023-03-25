#include "lve_pipeline.h"

#include "lve_model.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <errno.h>

typedef struct lvepili_shader_buf{
    int size;
    uint8_t buf[];
} lvepili_shader_buf;

// IMPORTANT: the return buffer for this HAS to be freed
lvepili_shader_buf* lvepili_read_file(const char* file_path){
    FILE *fp = fopen(file_path, "rb");
    if (fp == NULL) {
        printf("\033[0;31m[ERROR]\033[0m Could not read file '%s'\n", file_path);
        return NULL;
    }
    fseek(fp, 0L, SEEK_END);
    int sz = ftell(fp);
    rewind(fp);
    lvepili_shader_buf* r = malloc(sizeof(int) + sz * sizeof(uint8_t));
    r->size = sz;
    for(int i=0;i<sz-1;i++){
        r->buf[i]=fgetc(fp);
    }
    fclose(fp);

    return r;
}

void lvepili_bind(lve_pipeline* lvepili, VkCommandBuffer command_buffer){
    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, lvepili->m_graphics_pipeline);
}

void lvepili_create_shader_module(lve_pipeline* lvepili, const lvepili_shader_buf* code, VkShaderModule* shader_module){
    VkShaderModuleCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = code->size; // could need a +1
    create_info.pCode = (uint32_t*)code->buf; // there's an incredibly high chance this doesnt work
    if (vkCreateShaderModule(lvedev_device(lvepili->m_device), &create_info, NULL, shader_module) != VK_SUCCESS)
        printf("\033[0;31m[ERROR]\033[0m lvepili_create_shader_module() failed!\n");
}

void lvepili_create_graphics_pipeline(
    lve_pipeline* lvepili,
    const char* vert_file_path,
    const char* frag_file_path,
    const lve_pipeline_config_info* config_info)
{
    assert(config_info->pipeline_layout != VK_NULL_HANDLE &&
        "\033[0;31m[ERROR]\033[0m Cannot create graphics pipeline! no pipeline_layout provided in config_info\n");
    assert(config_info->render_pass != VK_NULL_HANDLE &&
        "\033[0;31m[ERROR]\033[0m Cannot create graphics pipeline! no render_pass provided in config_info\n");

    lvepili_shader_buf* vert_code = lvepili_read_file(vert_file_path);
    lvepili_shader_buf* frag_code = lvepili_read_file(frag_file_path);
    printf("\033[0;34m[INFO]\033[0m Vert shader size: %d\n", vert_code->size);
    printf("\033[0;34m[INFO]\033[0m Frag shader size: %d\n", frag_code->size);

    lvepili_create_shader_module(lvepili, vert_code, &lvepili->m_vert_shader_module);
    lvepili_create_shader_module(lvepili, frag_code, &lvepili->m_frag_shader_module);

    VkPipelineShaderStageCreateInfo shader_stages[2] = {};

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

    uint32_t binding_descriptions_c;
    VkVertexInputBindingDescription* binding_descriptions = lvevtx_get_binding_descriptions(&binding_descriptions_c);

    uint32_t attribute_descriptions_c;
    VkVertexInputAttributeDescription* attribute_descriptions = lvevtx_get_attribute_descriptions(&attribute_descriptions_c);

    VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
    vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_info.vertexAttributeDescriptionCount = attribute_descriptions_c;
    vertex_input_info.vertexBindingDescriptionCount = binding_descriptions_c;
    vertex_input_info.pVertexAttributeDescriptions = attribute_descriptions;
    vertex_input_info.pVertexBindingDescriptions = binding_descriptions;

    VkGraphicsPipelineCreateInfo pipeline_info = {};
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
    pipeline_info.pDynamicState = &config_info->dynamic_state_info;

    pipeline_info.layout = config_info->pipeline_layout;
    pipeline_info.renderPass = config_info->render_pass;
    pipeline_info.subpass = config_info->subpass;

    pipeline_info.basePipelineIndex = -1;
    pipeline_info.basePipelineHandle = VK_NULL_HANDLE;

    //this generates a warning in the frag shader that says theres no
    //matching output for the fragment shader (location 0).
    //this may be because of the lack of a swap chain, but otherwise
    //it's caused due to misconfiguration of lve_device
    if(vkCreateGraphicsPipelines(
        lvedev_device(lvepili->m_device),
        VK_NULL_HANDLE,
        1,
        &pipeline_info,
        NULL,
        &lvepili->m_graphics_pipeline) != VK_SUCCESS)
        printf("\033[0;31m[ERROR]\033[0m vkCreateGraphicsPipelines() failed!(good luck fixing this lmao)\n");

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
    memset(r, 0, sizeof(lve_pipeline));

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

void lvepili_default_pipeline_config_info(lve_pipeline_config_info* config_info)
{
    //lve_pipeline_config_info* r = malloc(sizeof(lve_pipeline_config_info));
    //memset(r, 0, sizeof(lve_pipeline_config_info));
    *config_info = (lve_pipeline_config_info){};

    config_info->input_assembly_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    config_info->input_assembly_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    config_info->input_assembly_info.primitiveRestartEnable = VK_FALSE;

    config_info->viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    config_info->viewport_info.viewportCount = 1;
    config_info->viewport_info.pViewports = NULL;
    config_info->viewport_info.scissorCount = 1;
    config_info->viewport_info.pScissors = NULL;

    config_info->rasterization_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    config_info->rasterization_info.depthClampEnable = VK_FALSE;
    config_info->rasterization_info.rasterizerDiscardEnable = VK_FALSE;
    config_info->rasterization_info.polygonMode = VK_POLYGON_MODE_FILL;
    config_info->rasterization_info.lineWidth = 1.0f;
    config_info->rasterization_info.cullMode = VK_CULL_MODE_NONE;
    config_info->rasterization_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
    config_info->rasterization_info.depthBiasEnable = VK_FALSE;
    config_info->rasterization_info.depthBiasConstantFactor = 0.0f;  // Optional
    config_info->rasterization_info.depthBiasClamp = 0.0f;           // Optional
    config_info->rasterization_info.depthBiasSlopeFactor = 0.0f;     // Optional

    config_info->multisample_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    config_info->multisample_info.sampleShadingEnable = VK_FALSE;
    config_info->multisample_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    config_info->multisample_info.minSampleShading = 1.0f;           // Optional
    config_info->multisample_info.pSampleMask = NULL;             // Optional
    config_info->multisample_info.alphaToCoverageEnable = VK_FALSE;  // Optional
    config_info->multisample_info.alphaToOneEnable = VK_FALSE;       // Optional

    config_info->color_blend_attachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
        VK_COLOR_COMPONENT_A_BIT;
    config_info->color_blend_attachment.blendEnable = VK_FALSE;
    config_info->color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
    config_info->color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
    config_info->color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;              // Optional
    config_info->color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
    config_info->color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
    config_info->color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;              // Optional

    config_info->color_blend_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    config_info->color_blend_info.logicOpEnable = VK_FALSE;
    config_info->color_blend_info.logicOp = VK_LOGIC_OP_COPY;  // Optional
    config_info->color_blend_info.attachmentCount = 1;
    config_info->color_blend_info.pAttachments = &config_info->color_blend_attachment;
    config_info->color_blend_info.blendConstants[0] = 0.0f;  // Optional
    config_info->color_blend_info.blendConstants[1] = 0.0f;  // Optional
    config_info->color_blend_info.blendConstants[2] = 0.0f;  // Optional
    config_info->color_blend_info.blendConstants[3] = 0.0f;  // Optional

    config_info->depth_stencil_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    config_info->depth_stencil_info.depthTestEnable = VK_TRUE;
    config_info->depth_stencil_info.depthWriteEnable = VK_TRUE;
    config_info->depth_stencil_info.depthCompareOp = VK_COMPARE_OP_LESS;
    config_info->depth_stencil_info.depthBoundsTestEnable = VK_FALSE;
    config_info->depth_stencil_info.minDepthBounds = 0.0f;  // Optional
    config_info->depth_stencil_info.maxDepthBounds = 1.0f;  // Optional
    config_info->depth_stencil_info.stencilTestEnable = VK_FALSE;
    config_info->depth_stencil_info.front = (VkStencilOpState){};  // Optional
    config_info->depth_stencil_info.back = (VkStencilOpState){};   // Optional

    config_info->pipeline_layout = NULL;
    config_info->render_pass = NULL;
    config_info->subpass = 0;

    //config_info->dynamic_state_enables = (VkDynamicState[]){VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    config_info->dynamic_state_enables[0] = VK_DYNAMIC_STATE_VIEWPORT;
    config_info->dynamic_state_enables[1] = VK_DYNAMIC_STATE_SCISSOR;
    config_info->dynamic_state_enables_c = sizeof(config_info->dynamic_state_enables) / sizeof(VkDynamicState);
    config_info->dynamic_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    config_info->dynamic_state_info.pDynamicStates = config_info->dynamic_state_enables;
    config_info->dynamic_state_info.dynamicStateCount = config_info->dynamic_state_enables_c;
    config_info->dynamic_state_info.flags = 0;
}

//theres a chance that this isnt working if something glitches
//out try checking here for errors