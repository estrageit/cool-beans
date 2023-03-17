#pragma once

#include "lve_window.h"
#include "lve_pipeline.h"

#define W_WIDTH 800
#define W_HEIGHT 600

typedef struct lve_first_app {
    lve_window* m_window;
    lve_pipeline* m_pipeline;
} lve_first_app;

void lvefirapp_run(lve_first_app* lvefirapp);
lve_first_app* lvefirapp_make();
void lvefirapp_destroy(lve_first_app* lvefirapp);