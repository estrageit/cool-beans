#include "lve_swap_chain.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

//these could be inline
VkFramebuffer lveswch_get_framebuffer(lve_swap_chain* lveswch, int index)
    {return lveswch->m_swap_chain_framebuffers[index];}
VkRenderPass lveswch_get_render_pass(lve_swap_chain* lveswch)
    {return lveswch->m_render_pass;}
VkImageView lveswch_get_image_view(lve_swap_chain* lveswch, int index)
    {return lveswch->m_depth_image_views[index];}
uint64_t lveswch_image_count(lve_swap_chain* lveswch)
    {return lveswch->m_swap_chain_images_c;}
VkFormat lveswch_get_swap_chain_image_format(lve_swap_chain* lveswch)
    {return lveswch->m_swap_chain_image_format;}
VkExtent2D lveswch_get_swap_chain_extent(lve_swap_chain* lveswch)
    {return lveswch->m_swap_chain_extent;}
uint32_t lveswch_width(lve_swap_chain* lveswch)
    {return lveswch->m_swap_chain_extent.width;}
uint32_t lveswch_height(lve_swap_chain* lveswch)
    {return lveswch->m_swap_chain_extent.height;}

float lveswch_extent_aspect_ratio(lve_swap_chain* lveswch)
    {return ((float)lveswch->m_swap_chain_extent.width)
    / ((float)lveswch->m_swap_chain_extent.height);}

VkSurfaceFormatKHR lveswch_choose_swap_surface_format(uint32_t available_formats_c,
      const VkSurfaceFormatKHR* available_formats)
{
    for (int i=0;i<available_formats_c;i++) {
        if (available_formats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
            available_formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return available_formats[i];
        }
    }
    return available_formats[0];
}

VkPresentModeKHR lveswch_choose_swap_present_mode(uint32_t available_present_modes_c,
      const VkPresentModeKHR* available_present_modes)
{
    for (int i=0;i<available_present_modes_c;i++) {
        if (available_present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
            printf("\033[0;34m[INFO]\033[0m Present mode: Mailbox\n");
            return available_present_modes[i];
        }
    }
    // for (const auto &availablePresentMode : availablePresentModes) {
    //   if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
    //     std::cout << "Present mode: Immediate" << std::endl;
    //     return availablePresentMode;
    //   }
    // }
    printf("\033[0;34m[INFO]\033[0m Present mode: V-Sync\n");
    return VK_PRESENT_MODE_FIFO_KHR;
}

uint32_t max(uint32_t a, uint32_t b){
    return ((a > b) * a) + ((a <= b) * b);
}

uint32_t min(uint32_t a, uint32_t b){
    return ((a > b) * b) + ((a <= b) * a);
}

VkExtent2D lveswch_choose_swap_extent
    (lve_swap_chain* lveswch, const VkSurfaceCapabilitiesKHR* capabilities)
{
    if (capabilities->currentExtent.width != UINT32_MAX) {
        return capabilities->currentExtent;
    } else {
        VkExtent2D actualExtent = lveswch->m_window_extent;;
        actualExtent.width = max(
            capabilities->minImageExtent.width,
            min(capabilities->maxImageExtent.width, actualExtent.width));
        actualExtent.height = max(
            capabilities->minImageExtent.height,
            min(capabilities->maxImageExtent.height, actualExtent.height));
        return actualExtent;
    }
}

void lveswch_create_swap_chain(lve_swap_chain* lveswch){
    lve_swap_chain_support_details swapChainSupport = lvedev_get_swap_chain_support(lveswch->m_device);
    VkSurfaceFormatKHR surfaceFormat = lveswch_choose_swap_surface_format(swapChainSupport.formats_count, swapChainSupport.formats);
    VkPresentModeKHR presentMode = lveswch_choose_swap_present_mode(swapChainSupport.present_modes_count, swapChainSupport.present_modes);
    VkExtent2D extent = lveswch_choose_swap_extent(lveswch, &swapChainSupport.capabilities);
    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 &&
        imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }
    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = lvedev_surface(lveswch->m_device);
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    lve_queue_family_indices indices = lvedev_find_physical_queue_families(lveswch->m_device);
    uint32_t queueFamilyIndices[] = {indices.graphics_family, indices.present_family};
    if (indices.graphics_family != indices.present_family) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;      // Optional
        createInfo.pQueueFamilyIndices = NULL;  // Optional
    }
    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;
    if (vkCreateSwapchainKHR(lvedev_device(lveswch->m_device), &createInfo, NULL, &lveswch->m_swap_chain) != VK_SUCCESS) {
        printf("\033[0;31m[ERROR]\033[0m Failed to create swap chain!\n");
        exit(-1);
    }
    // we only specified a minimum number of images in the swap chain, so the implementation is
    // allowed to create a swap chain with more. That's why we'll first query the final number of
    // images with vkGetSwapchainImagesKHR, then resize the container and finally call it again to
    // retrieve the handles.
    vkGetSwapchainImagesKHR(lvedev_device(lveswch->m_device), lveswch->m_swap_chain, &imageCount, NULL);
    lveswch->m_swap_chain_images = malloc(sizeof(VkImage) * imageCount);
    memset(lveswch->m_swap_chain_images, 0, sizeof(VkImage) * imageCount);
    lveswch->m_swap_chain_images_c = imageCount;
    vkGetSwapchainImagesKHR(lvedev_device(lveswch->m_device), lveswch->m_swap_chain, &imageCount, lveswch->m_swap_chain_images);
    lveswch->m_swap_chain_image_format = surfaceFormat.format;
    lveswch->m_swap_chain_extent = extent;
}

void lveswch_create_image_views(lve_swap_chain* lveswch){
    lveswch->m_swap_chain_image_views = malloc(sizeof(VkImageView) * lveswch->m_swap_chain_images_c);
    memset(lveswch->m_swap_chain_image_views, 0, sizeof(VkImageView) * lveswch->m_swap_chain_images_c);
    lveswch->m_swap_chain_image_views_c = lveswch->m_swap_chain_images_c;
    for (size_t i = 0; i < lveswch->m_swap_chain_images_c; i++) {
        VkImageViewCreateInfo viewInfo = {};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = lveswch->m_swap_chain_images[i];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = lveswch->m_swap_chain_image_format;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;
        if (vkCreateImageView(lvedev_device(lveswch->m_device), &viewInfo, NULL, lveswch->m_swap_chain_image_views + i) !=
            VK_SUCCESS) {
            printf("\033[0;31m[ERROR]\033[0m Failed to create texture image view!\n");
            exit(-1);
        }
    }
}

void lveswch_create_depth_resources(lve_swap_chain* lveswch){
    VkFormat depthFormat = lveswch_find_depth_format(lveswch);
    VkExtent2D swapChainExtent = lveswch_get_swap_chain_extent(lveswch);
    lveswch->m_depth_images = malloc(sizeof(VkImage) * lveswch_image_count(lveswch));
    memset(lveswch->m_depth_images, 0, sizeof(VkImage) * lveswch_image_count(lveswch));
    lveswch->m_depth_images_c = lveswch_image_count(lveswch);
    lveswch->m_depth_image_memories = malloc(sizeof(VkDeviceMemory) * lveswch_image_count(lveswch));
    memset(lveswch->m_depth_image_memories, 0, sizeof(VkDeviceMemory) * lveswch_image_count(lveswch));
    lveswch->m_depth_image_memories_c = lveswch_image_count(lveswch);
    lveswch->m_depth_image_views = malloc(sizeof(VkImageView) * lveswch_image_count(lveswch));
    memset(lveswch->m_depth_image_views, 0, sizeof(VkImageView) * lveswch_image_count(lveswch));
    lveswch->m_depth_image_views_c = lveswch_image_count(lveswch);
    for (int i = 0; i < lveswch->m_depth_images_c; i++) {
        VkImageCreateInfo imageInfo = {};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = swapChainExtent.width;
        imageInfo.extent.height = swapChainExtent.height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = depthFormat;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.flags = 0;
        lvedev_create_image_with_info(
            lveswch->m_device,
            &imageInfo,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            lveswch->m_depth_images + i,
            lveswch->m_depth_image_memories + i);
        VkImageViewCreateInfo viewInfo = {};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = lveswch->m_depth_images[i];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = depthFormat;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;
        if (vkCreateImageView(lvedev_device(lveswch->m_device), &viewInfo, NULL, lveswch->m_depth_image_views + i) != VK_SUCCESS) {
            printf("\033[0;31m[ERROR]\033[0m Failed to create texture image view!\n");
            exit(-1);
        }
    }
}

void lveswch_create_render_pass(lve_swap_chain* lveswch){
    VkAttachmentDescription depthAttachment = {};
    depthAttachment.format = lveswch_find_depth_format(lveswch);
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    VkAttachmentReference depthAttachmentRef = {};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = lveswch_get_swap_chain_image_format(lveswch);
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;
    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.srcAccessMask = 0;
    dependency.srcStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstSubpass = 0;
    dependency.dstStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask =
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    VkAttachmentDescription attachments[2] = {colorAttachment, depthAttachment};
    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 2;
    renderPassInfo.pAttachments = attachments;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;
    if (vkCreateRenderPass(lvedev_device(lveswch->m_device), &renderPassInfo, NULL, &lveswch->m_render_pass) != VK_SUCCESS) {
        printf("\033[0;31m[ERROR]\033[0m Failed to create render pass!\n");
        exit(-1);
    }
}

void lveswch_create_framebuffers(lve_swap_chain* lveswch){
    lveswch->m_swap_chain_framebuffers = malloc(sizeof(VkFramebuffer) * lveswch_image_count(lveswch));
    memset(lveswch->m_swap_chain_framebuffers, 0, sizeof(VkFramebuffer) * lveswch_image_count(lveswch));
    lveswch->m_swap_chain_framebuffers_c = lveswch_image_count(lveswch);
    for (size_t i = 0; i < lveswch_image_count(lveswch); i++) {
        //idk if this array is working properly
        VkImageView attachments[2] = {lveswch->m_swap_chain_image_views[i], lveswch->m_depth_image_views[i]};
        VkExtent2D swapChainExtent = lveswch_get_swap_chain_extent(lveswch);
        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = lveswch->m_render_pass;
        framebufferInfo.attachmentCount = 2;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = swapChainExtent.width;
        framebufferInfo.height = swapChainExtent.height;
        framebufferInfo.layers = 1;
        if (vkCreateFramebuffer(
            lvedev_device(lveswch->m_device),
            &framebufferInfo,
            NULL,
            lveswch->m_swap_chain_framebuffers + i) != VK_SUCCESS) {
            printf("\033[0;31m[ERROR]\033[0m Failed to create framebuffer!\n");
            exit(-1);
        }
    }
}

void lveswch_create_sync_objects(lve_swap_chain* lveswch){
    lveswch->m_image_available_semaphores = malloc(sizeof(VkSemaphore) * LVESWCH_MAX_FRAMES_IN_FLIGHT);
    memset(lveswch->m_image_available_semaphores, 0, sizeof(VkSemaphore) * LVESWCH_MAX_FRAMES_IN_FLIGHT);
    lveswch->m_image_available_semaphores_c = LVESWCH_MAX_FRAMES_IN_FLIGHT;
    lveswch->m_render_finished_semaphores = malloc(sizeof(VkSemaphore) * LVESWCH_MAX_FRAMES_IN_FLIGHT);
    memset(lveswch->m_render_finished_semaphores, 0, sizeof(VkSemaphore) * LVESWCH_MAX_FRAMES_IN_FLIGHT);
    lveswch->m_render_finished_semaphores_c = LVESWCH_MAX_FRAMES_IN_FLIGHT;
    lveswch->m_in_flight_fences = malloc(sizeof(VkFence) * LVESWCH_MAX_FRAMES_IN_FLIGHT);
    memset(lveswch->m_in_flight_fences, 0, sizeof(VkFence) * LVESWCH_MAX_FRAMES_IN_FLIGHT);
    lveswch->m_in_flight_fences_c = LVESWCH_MAX_FRAMES_IN_FLIGHT;
    lveswch->m_images_in_flight = malloc(sizeof(VkFence) * lveswch_image_count(lveswch));
    memset(lveswch->m_images_in_flight, 0, sizeof(VkFence) * lveswch_image_count(lveswch));
    lveswch->m_images_in_flight_c = lveswch_image_count(lveswch);
    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    for (size_t i = 0; i < LVESWCH_MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(lvedev_device(lveswch->m_device), &semaphoreInfo, NULL, lveswch->m_image_available_semaphores + i) !=
                VK_SUCCESS ||
            vkCreateSemaphore(lvedev_device(lveswch->m_device), &semaphoreInfo, NULL, lveswch->m_render_finished_semaphores + i) !=
                VK_SUCCESS ||
            vkCreateFence(lvedev_device(lveswch->m_device), &fenceInfo, NULL, lveswch->m_in_flight_fences + i) != VK_SUCCESS) {
            printf("\033[0;31m[ERROR]\033[0m Failed to create synchronization objects for a frame!\n");
            exit(-1);
        }
    }
}

VkFormat lveswch_find_depth_format(lve_swap_chain* lveswch){
    const VkFormat candidates[] =
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT};
    return lvedev_find_supported_format(
        lveswch->m_device,
        3,
        candidates,
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

VkResult lveswch_acquire_next_image(lve_swap_chain* lveswch, uint32_t* image_index){
        vkWaitForFences(
        lvedev_device(lveswch->m_device),
        1,
        lveswch->m_in_flight_fences + lveswch->m_current_frame,
        VK_TRUE,
        UINT64_MAX);
    VkResult result = vkAcquireNextImageKHR(
        lvedev_device(lveswch->m_device),
        lveswch->m_swap_chain,
        UINT64_MAX,
        lveswch->m_image_available_semaphores[lveswch->m_current_frame],  // must be a not signaled semaphore
        VK_NULL_HANDLE,
        image_index);
    return result;
}

VkResult lveswch_submit_command_buffers
    (lve_swap_chain* lveswch, const VkCommandBuffer *buffers, uint32_t *image_index)
{
    if (lveswch->m_images_in_flight[*image_index] != VK_NULL_HANDLE) {
        vkWaitForFences(lvedev_device(lveswch->m_device), 1, &lveswch->m_images_in_flight[*image_index], VK_TRUE, UINT64_MAX);
    }
    lveswch->m_images_in_flight[*image_index] = lveswch->m_in_flight_fences[lveswch->m_current_frame];
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    VkSemaphore waitSemaphores[] = {lveswch->m_image_available_semaphores[lveswch->m_current_frame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = buffers;
    VkSemaphore signalSemaphores[] = {lveswch->m_render_finished_semaphores[lveswch->m_current_frame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;
    vkResetFences(lvedev_device(lveswch->m_device), 1, &lveswch->m_in_flight_fences[lveswch->m_current_frame]);
    if (vkQueueSubmit(lvedev_graphics_queue(lveswch->m_device), 1, &submitInfo, lveswch->m_in_flight_fences[lveswch->m_current_frame]) !=
        VK_SUCCESS) {
        printf("\033[0;31m[ERROR]\033[0m Failed to submit draw command buffer!\n");
        exit(-1);
    }
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    VkSwapchainKHR swapChains[] = {lveswch->m_swap_chain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = image_index;
    VkResult result = vkQueuePresentKHR(lvedev_present_queue(lveswch->m_device), &presentInfo);
    lveswch->m_current_frame = (lveswch->m_current_frame + 1) % LVESWCH_MAX_FRAMES_IN_FLIGHT;
    return result;
}

lve_swap_chain* lveswch_make(lve_device* device_ptr, VkExtent2D extent){
    lve_swap_chain* r = malloc(sizeof(lve_swap_chain));
    memset(r, 0, sizeof(lve_swap_chain));
    r->m_device = device_ptr;
    r->m_window_extent = extent;
    lveswch_create_swap_chain(r);
    lveswch_create_image_views(r);
    lveswch_create_depth_resources(r);
    lveswch_create_render_pass(r);
    lveswch_create_framebuffers(r);
    lveswch_create_sync_objects(r);
    return r;
}

void lveswch_destroy(lve_swap_chain* lveswch){
    for (int i=0;i<lveswch->m_swap_chain_image_views_c;i++){
        vkDestroyImageView(lvedev_device(lveswch->m_device), lveswch->m_swap_chain_image_views[i], NULL);
    }
    free(lveswch->m_swap_chain_image_views);
    lveswch->m_swap_chain_image_views = NULL;
    lveswch->m_swap_chain_image_views_c = 0;
    if (lveswch->m_swap_chain != NULL) {
        vkDestroySwapchainKHR(lvedev_device(lveswch->m_device), lveswch->m_swap_chain, NULL);
        lveswch->m_swap_chain = NULL;
    }
    for (int i = 0; i < lveswch->m_depth_images_c; i++) {
        vkDestroyImageView(lvedev_device(lveswch->m_device), lveswch->m_depth_image_views[i], NULL);
        vkDestroyImage(lvedev_device(lveswch->m_device), lveswch->m_depth_images[i], NULL);
        vkFreeMemory(lvedev_device(lveswch->m_device), lveswch->m_depth_image_memories[i], NULL);
    }
    for (int i=0;i<lveswch->m_swap_chain_framebuffers_c;i++){
        vkDestroyFramebuffer(lvedev_device(lveswch->m_device), lveswch->m_swap_chain_framebuffers[i], NULL);
    }
    vkDestroyRenderPass(lvedev_device(lveswch->m_device), lveswch->m_render_pass, NULL);
    // cleanup synchronization objects
    for (size_t i = 0; i < LVESWCH_MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(lvedev_device(lveswch->m_device),lveswch->m_render_finished_semaphores[i], NULL);
        vkDestroySemaphore(lvedev_device(lveswch->m_device),lveswch->m_image_available_semaphores[i], NULL);
        vkDestroyFence(lvedev_device(lveswch->m_device),lveswch->m_in_flight_fences[i], NULL);
    }
    free(lveswch);
}