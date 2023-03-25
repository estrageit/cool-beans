#include "lve_model.h"

#include <assert.h>
#include <string.h>

void lvemdl_create_vertex_buffers(
    lve_model* lvemdl, lve_vertex* vertices, uint32_t vertices_c){
    lvemdl->m_vertex_count = vertices_c;
    assert(lvemdl->m_vertex_count >= 3 && "[ERROR] vertex count is less than 3\n");
    VkDeviceSize buffer_size = sizeof(lve_vertex) * lvemdl->m_vertex_count;
    lvedev_create_buffer(
        lvemdl->m_device, 
        buffer_size,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &lvemdl->m_vertex_buffer,
        &lvemdl->m_vertex_buffer_memory);
    void* data;
    vkMapMemory(
        lvedev_device(lvemdl->m_device), 
        lvemdl->m_vertex_buffer_memory, 
        0, 
        buffer_size, 
        0, 
        &data);
    memcpy(data, vertices, buffer_size);
    vkUnmapMemory(
        lvedev_device(lvemdl->m_device), 
        lvemdl->m_vertex_buffer_memory);
}

void lvemdl_bind(lve_model* lvemdl, VkCommandBuffer command_buffer){
    VkBuffer buffers[] = {lvemdl->m_vertex_buffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(command_buffer, 0, 1, buffers, offsets);
}

void lvemdl_draw(lve_model* lvemdl, VkCommandBuffer command_buffer){
    vkCmdDraw(command_buffer, lvemdl->m_vertex_count, 1, 0, 0);
}

void lvemdl_change_buf(lve_model* lvemdl, int v_index, lve_vertex value)
{
    VkDeviceSize buffer_size = sizeof(lve_vertex) * lvemdl->m_vertex_count;
    void* data;
    vkMapMemory(
        lvedev_device(lvemdl->m_device), 
        lvemdl->m_vertex_buffer_memory, 
        0, 
        buffer_size,
        0, 
        &data);
    //memcpy(data, vertices, buffer_size);
    ((lve_vertex*)data)[v_index] = value;
    vkUnmapMemory(
        lvedev_device(lvemdl->m_device), 
        lvemdl->m_vertex_buffer_memory);
}

VkVertexInputBindingDescription* lvevtx_get_binding_descriptions(
    uint32_t* count)
{
    *count = 1;
    VkVertexInputBindingDescription* r = malloc(sizeof(VkVertexInputBindingDescription) * *count);
    memset(r, 0, sizeof(VkVertexInputBindingDescription) * *count);
    r[0].binding = 0;
    r[0].stride = sizeof(lve_vertex);
    r[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return r;
}

VkVertexInputAttributeDescription* lvevtx_get_attribute_descriptions(
    uint32_t* count)
{
    *count = 2; // number of attributes
    VkVertexInputAttributeDescription* r = malloc(sizeof(VkVertexInputAttributeDescription) * *count);
    memset(r, 0, sizeof(VkVertexInputAttributeDescription) * *count);
    r[0].binding = 0;
    r[0].location = 0;
    r[0].format = VK_FORMAT_R32G32_SFLOAT;
    r[0].offset = 0;

    r[1].binding = 0;
    r[1].location = 1;
    r[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    r[1].offset = offsetof(lve_vertex, color);
    return r;
}

lve_model* lvemdl_make(
    lve_device* device, lve_vertex* vertices, uint32_t vertices_c)
{
    lve_model* r = malloc(sizeof(lve_model));
    *r = (lve_model){};
    r->m_device = device;
    lvemdl_create_vertex_buffers(r, vertices, vertices_c);
    return r;
}

void lvemdl_destroy(lve_model* lvemdl){
    vkDestroyBuffer(lvedev_device(lvemdl->m_device), lvemdl->m_vertex_buffer, NULL);
    vkFreeMemory(lvedev_device(lvemdl->m_device), lvemdl->m_vertex_buffer_memory, NULL);
    free(lvemdl);
}