#include "first_app.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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
    lve_pipeline_config_info* pipeline_config = lvepili_default_pipeline_config_info(lveswch_width(firapp->m_swap_chain), lveswch_height(firapp->m_swap_chain));
    pipeline_config->render_pass = lveswch_get_render_pass(firapp->m_swap_chain);
    pipeline_config->pipeline_layout = firapp->m_pipeline_layout;
    firapp->m_pipeline = lvepili_make(
        firapp->m_device,
        "shaders/simple_shader.vert.spv",
        "shaders/simple_shader.frag.spv",
        pipeline_config);
    free(pipeline_config);
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
    for(int i=0;i<firapp->m_command_buffers_c;i++){
        VkCommandBufferBeginInfo begin_info = {};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        if(vkBeginCommandBuffer(firapp->m_command_buffers[i], &begin_info) != VK_SUCCESS){
            printf("\033[0;31m[ERROR]\033[0m Failed to begin recording command buffer!\n");
            exit(-1);
        }
        VkRenderPassBeginInfo render_pass_info = {};
        render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_info.renderPass = lveswch_get_render_pass(firapp->m_swap_chain);
        render_pass_info.framebuffer = lveswch_get_framebuffer(firapp->m_swap_chain, i);

        render_pass_info.renderArea.offset = (VkOffset2D){0,0};
        render_pass_info.renderArea.extent = lveswch_get_swap_chain_extent(firapp->m_swap_chain);

        VkClearValue clear_values[2];
        clear_values[0].color = (VkClearColorValue){{0.1f, 0.1f, 0.2f, 1.0f}};
        clear_values[1].depthStencil = (VkClearDepthStencilValue){1.0f, 0};

        render_pass_info.clearValueCount = sizeof(clear_values) / sizeof(VkClearValue);
        render_pass_info.pClearValues = clear_values;

        vkCmdBeginRenderPass(firapp->m_command_buffers[i], &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

        lvepili_bind(firapp->m_pipeline, firapp->m_command_buffers[i]);
        //vkCmdDraw(firapp->m_command_buffers[i], 3, 1, 0, 0);
        lvemdl_bind(firapp->m_model, firapp->m_command_buffers[i]);
        lvemdl_draw(firapp->m_model, firapp->m_command_buffers[i]);

        vkCmdEndRenderPass(firapp->m_command_buffers[i]);
        if(vkEndCommandBuffer(firapp->m_command_buffers[i]) != VK_SUCCESS){
            printf("\033[0;31m[ERROR]\033[0m Failed to end recording command buffer!\n");
            exit(-1);
        }
    }
}

void firapp_draw_frame(first_app* firapp){
    uint32_t image_index;
    VkResult result = lveswch_acquire_next_image(firapp->m_swap_chain, &image_index);

    //lvemdl_change_buf(firapp->m_model, 0, (lve_vertex){{0.5, 0.5 + glfwGetTime() / 100}});

    if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR){
        printf("\033[0;31m[ERROR]\033[0m Failed to acquire next swap chain image!\n");
        //exit(-1);
    }

    result = lveswch_submit_command_buffers(firapp->m_swap_chain, firapp->m_command_buffers + image_index, &image_index);
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
    r->m_swap_chain = lveswch_make(r->m_device, lvewin_get_extent(r->m_window));
    firapp_load_models(r);
    firapp_create_pipeline_layout(r);
    firapp_create_pipeline(r);
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