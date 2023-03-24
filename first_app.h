#pragma once

#include "lve_window.h"
#include "lve_pipeline.h"
#include "lve_device.h"
#include "lve_swap_chain.h"

#define W_WIDTH 800
#define W_HEIGHT 600

typedef struct first_app {
    lve_window* m_window;
    lve_pipeline* m_pipeline;// supposed to be a unique pointer
    lve_device* m_device;
    lve_swap_chain* m_swap_chain;
    VkPipelineLayout m_pipeline_layout;
    VkCommandBuffer* m_command_buffers;
    uint32_t m_command_buffers_c;
} first_app;

void firapp_run(first_app* lvefirapp);
first_app* firapp_make();
void firapp_destroy(first_app* lvefirapp);