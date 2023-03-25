#include "first_app.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

void firapp_record_command_buffer(first_app* firapp, int image_index){
    VkCommandBufferBeginInfo begin_info = {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    if(vkBeginCommandBuffer(firapp->m_command_buffers[image_index], &begin_info) != VK_SUCCESS){
        printf("\033[0;31m[ERROR]\033[0m Failed to begin recording command buffer!\n");
        exit(-1);
    }
    VkRenderPassBeginInfo render_pass_info = {};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = lveswch_get_render_pass(firapp->m_swap_chain);
    render_pass_info.framebuffer = lveswch_get_framebuffer(firapp->m_swap_chain, image_index);

    render_pass_info.renderArea.offset = (VkOffset2D){0,0};
    render_pass_info.renderArea.extent = lveswch_get_swap_chain_extent(firapp->m_swap_chain);

    VkClearValue clear_values[2];
    clear_values[0].color = (VkClearColorValue){{0.1f, 0.1f, 0.2f, 1.0f}};
    clear_values[1].depthStencil = (VkClearDepthStencilValue){1.0f, 0};

    render_pass_info.clearValueCount = sizeof(clear_values) / sizeof(VkClearValue);
    render_pass_info.pClearValues = clear_values;

    vkCmdBeginRenderPass(firapp->m_command_buffers[image_index], &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = lveswch_get_swap_chain_extent(firapp->m_swap_chain).width;
    viewport.height = lveswch_get_swap_chain_extent(firapp->m_swap_chain).height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    VkRect2D scissor = {{0, 0}, lveswch_get_swap_chain_extent(firapp->m_swap_chain)};
    vkCmdSetViewport(firapp->m_command_buffers[image_index], 0, 1, &viewport);
    vkCmdSetScissor(firapp->m_command_buffers[image_index], 0, 1, &scissor);

    lvepili_bind(firapp->m_pipeline, firapp->m_command_buffers[image_index]);

    lvemdl_bind(firapp->m_model, firapp->m_command_buffers[image_index]);
    lvemdl_draw(firapp->m_model, firapp->m_command_buffers[image_index]);

    vkCmdEndRenderPass(firapp->m_command_buffers[image_index]);
    if(vkEndCommandBuffer(firapp->m_command_buffers[image_index]) != VK_SUCCESS){
        printf("\033[0;31m[ERROR]\033[0m Failed to end recording command buffer!\n");
        exit(-1);
    }
}

void firapp_free_command_buffers(first_app* firapp){
    vkFreeCommandBuffers(
        lvedev_device(firapp->m_device),
        firapp->m_device->m_command_pool,
        firapp->m_command_buffers_c,
        firapp->m_command_buffers);
    free(firapp->m_command_buffers);
}

void firapp_load_models(first_app* firapp){
    lve_vertex vertices[] = {
        {{ 0.5f,  0.5f}, {1, 1, 0}},
        {{ 0.5f, -0.5f}, {1, 0, 1}},
        {{-0.5f, -0.5f}, {0, 1, 1}}
    };
    firapp->m_model = lvemdl_make(firapp->m_device, vertices, sizeof(vertices) / sizeof(lve_vertex));
}

void firapp_create_pipeline_layout(first_app* firapp){
    VkPipelineLayoutCreateInfo pipeline_layout_info = {};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = 0;
    pipeline_layout_info.pSetLayouts = NULL;
    pipeline_layout_info.pushConstantRangeCount = 0;
    pipeline_layout_info.pPushConstantRanges = NULL;
    if (vkCreatePipelineLayout(
        lvedev_device(firapp->m_device),
        &pipeline_layout_info,
        NULL,
        &firapp->m_pipeline_layout) != VK_SUCCESS){
        printf("\033[0;31m[ERROR]\033[0m Failed to create pipeline layout!\n");
        exit(-1);
    }
}

void firapp_create_pipeline(first_app* firapp){
    assert(firapp->m_swap_chain != NULL && "Cannot create pipeline before swap chain");
    assert(firapp->m_pipeline_layout != NULL && "Cannot create pipeline before pipeline layout");

    lve_pipeline_config_info pipeline_config = {};
    lvepili_default_pipeline_config_info(&pipeline_config);
    pipeline_config.render_pass = lveswch_get_render_pass(firapp->m_swap_chain);
    pipeline_config.pipeline_layout = firapp->m_pipeline_layout;
    firapp->m_pipeline = lvepili_make(
        firapp->m_device,
        "shaders/simple_shader.vert.spv",
        "shaders/simple_shader.frag.spv",
        &pipeline_config);
}

void firapp_create_command_buffers(first_app* firapp){
    firapp->m_command_buffers_c = lveswch_image_count(firapp->m_swap_chain);
    firapp->m_command_buffers = malloc(sizeof(VkCommandBuffer) * firapp->m_command_buffers_c);
    memset(firapp->m_command_buffers, 0, sizeof(VkCommandBuffer) * firapp->m_command_buffers_c);

    VkCommandBufferAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandPool = lvedev_get_command_pool(firapp->m_device);
    alloc_info.commandBufferCount = (uint32_t)firapp->m_command_buffers_c;
    if(vkAllocateCommandBuffers(lvedev_device(firapp->m_device), &alloc_info, firapp->m_command_buffers) != VK_SUCCESS){
        printf("\033[0;31m[ERROR]\033[0m Failed to allocate command buffers!\n");
        exit(-1);
    }
}

void firapp_recreate_swap_chain(first_app* firapp){
    VkExtent2D extent = lvewin_get_extent(firapp->m_window);
    while (extent.width == 0 || extent.height == 0){
        extent = lvewin_get_extent(firapp->m_window);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(lvedev_device(firapp->m_device));
    if(firapp->m_swap_chain != NULL){
        //lveswch_destroy(firapp->m_swap_chain);
        //firapp->m_swap_chain = NULL;
        firapp->m_swap_chain = lveswch_make_from_previous(firapp->m_device, extent, firapp->m_swap_chain);
        if(lveswch_image_count(firapp->m_swap_chain) != firapp->m_command_buffers_c){
            firapp_free_command_buffers(firapp);
            firapp_create_command_buffers(firapp);
        }
    } else
        firapp->m_swap_chain = lveswch_make(firapp->m_device, extent);
    if(firapp->m_pipeline != NULL){
        lvepili_destroy(firapp->m_pipeline);
        firapp->m_pipeline = NULL;
    }

    //todo: check if old render pass is compatible :)
    firapp_create_pipeline(firapp);
}

void firapp_draw_frame(first_app* firapp){
    uint32_t image_index;
    VkResult result = lveswch_acquire_next_image(firapp->m_swap_chain, &image_index);

    //vkDeviceWaitIdle(lvedev_device(firapp->m_device));
    //lvemdl_change_buf(firapp->m_model, 1, (lve_vertex){{0.5, - 0.5 + glfwGetTime() / 100}});

    if(result == VK_ERROR_OUT_OF_DATE_KHR){
        firapp_recreate_swap_chain(firapp);
        return;
    }
    if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR){
        printf("\033[0;31m[ERROR]\033[0m Failed to acquire next swap chain image!\n");
        //exit(-1);
    }

    firapp_record_command_buffer(firapp, image_index);
    result = lveswch_submit_command_buffers(firapp->m_swap_chain, firapp->m_command_buffers + image_index, &image_index);
    if(result == VK_ERROR_OUT_OF_DATE_KHR ||
        result == VK_SUBOPTIMAL_KHR ||
        lvewin_was_window_resized(firapp->m_window)){
        firapp_recreate_swap_chain(firapp);
        return;
    }
    if(result != VK_SUCCESS){
        printf("\033[0;31m[ERROR]\033[0m Failed to present swap chain image!\n");
        exit(-1);
    }
}

void firapp_run(first_app* firapp){
    while(!lvewin_shouldclose(firapp->m_window)){
        glfwPollEvents();
        firapp_draw_frame(firapp);
    }
    vkDeviceWaitIdle(lvedev_device(firapp->m_device));
}

first_app* firapp_make(){
    first_app* r = malloc(sizeof(first_app));
    r->m_window = lvewin_make(W_WIDTH, W_HEIGHT, "COOL!");
    r->m_device = lvedev_make(r->m_window);
    //r->m_swap_chain = lveswch_make(r->m_device, lvewin_get_extent(r->m_window));
    firapp_load_models(r);
    firapp_create_pipeline_layout(r);
    firapp_recreate_swap_chain(r);
    firapp_create_command_buffers(r);
    return r;
}

void firapp_destroy(first_app* firapp){
    lvemdl_destroy(firapp->m_model);
    vkDestroyPipelineLayout(lvedev_device(firapp->m_device), firapp->m_pipeline_layout, NULL);
    lvepili_destroy(firapp->m_pipeline);
    lveswch_destroy(firapp->m_swap_chain);
    lvedev_destroy(firapp->m_device);
    lvewin_destroy(firapp->m_window);
    free(firapp);
}