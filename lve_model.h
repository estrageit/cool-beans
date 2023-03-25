#pragma once

#include "lve_device.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <cglm/cglm.h>

typedef struct lve_vertex{
    vec2 position;
    vec3 color;
} lve_vertex;

VkVertexInputBindingDescription* lvevtx_get_binding_descriptions(
    uint32_t* count);
VkVertexInputAttributeDescription* lvevtx_get_attribute_descriptions(
    uint32_t* count);

typedef struct lve_model{
    lve_device* m_device;
    VkBuffer m_vertex_buffer;
    VkDeviceMemory m_vertex_buffer_memory;
    uint32_t m_vertex_count;
} lve_model;
 
lve_model* lvemdl_make(
    lve_device* device, lve_vertex* vertices, uint32_t vertices_c);
void lvemdl_destroy(lve_model* lvemdl);

void lvemdl_bind(lve_model* lvemdl, VkCommandBuffer command_buffer);
void lvemdl_draw(lve_model* lvemdl, VkCommandBuffer command_buffer);


void lvemdl_change_buf(lve_model* lvemdl, int v_index, lve_vertex value);