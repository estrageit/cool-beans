#pragma once

#include "lve_device.h"

#include <vulkan/vulkan.h>

#define LVESWCH_MAX_FRAMES_IN_FLIGHT 2

typedef struct lve_swap_chain{
    VkFormat m_swap_chain_image_format;
    VkExtent2D m_swap_chain_extent;

    VkFramebuffer* m_swap_chain_framebuffers;
    uint32_t m_swap_chain_framebuffers_c;
    VkRenderPass m_render_pass;

    VkImage* m_depth_images;
    uint32_t m_depth_images_c;
    VkDeviceMemory* m_depth_image_memories;
    uint32_t m_depth_image_memories_c;
    VkImageView* m_depth_image_views;
    uint32_t m_depth_image_views_c;
    VkImage* m_swap_chain_images;
    uint32_t m_swap_chain_images_c;
    VkImageView* m_swap_chain_image_views;
    uint32_t m_swap_chain_image_views_c;

    lve_device* m_device;
    VkExtent2D m_window_extent;

    VkSwapchainKHR m_swap_chain;
    struct lve_swap_chain* old_swap_chain;

    VkSemaphore* m_image_available_semaphores;
    uint32_t m_image_available_semaphores_c;
    VkSemaphore* m_render_finished_semaphores;
    uint32_t m_render_finished_semaphores_c;
    VkFence* m_in_flight_fences;
    uint32_t m_in_flight_fences_c;
    VkFence* m_images_in_flight;
    uint32_t m_images_in_flight_c;
    uint64_t m_current_frame;
} lve_swap_chain;

lve_swap_chain* lveswch_make(lve_device* device, VkExtent2D extent);
lve_swap_chain* lveswch_make_from_previous(lve_device* device, VkExtent2D extent, lve_swap_chain* previous);

void lveswch_destroy(lve_swap_chain* lveswch);

VkFramebuffer lveswch_get_framebuffer(lve_swap_chain* lveswch, int index);
VkRenderPass lveswch_get_render_pass(lve_swap_chain* lveswch);
VkImageView lveswch_get_image_view(lve_swap_chain* lveswch, int index);
uint64_t lveswch_image_count(lve_swap_chain* lveswch);
VkFormat lveswch_get_swap_chain_image_format(lve_swap_chain* lveswch);
VkExtent2D lveswch_get_swap_chain_extent(lve_swap_chain* lveswch);
uint32_t lveswch_width(lve_swap_chain* lveswch);
uint32_t lveswch_height(lve_swap_chain* lveswch);

float lveswch_extent_aspect_ratio(lve_swap_chain* lveswch);

VkFormat lveswch_find_depth_format(lve_swap_chain* lveswch);

VkResult lveswch_acquire_next_image(lve_swap_chain* lveswch, uint32_t* image_index);
VkResult lveswch_submit_command_buffers
    (lve_swap_chain* lveswch, const VkCommandBuffer *buffers, uint32_t *image_index);