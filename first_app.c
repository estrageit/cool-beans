#include "first_app.h"

#include <stdlib.h>

lve_first_app* lvefirapp_make(){
    lve_first_app* r = malloc(sizeof(lve_first_app));
    r->m_window = lvewin_make(W_WIDTH, W_HEIGHT, "COOL!");
    r->m_pipeline = lvepili_make(
        "shaders/simple_shader.vert.spv",
        "shaders/simple_shader.frag.spv");
    return r;
}

void lvefirapp_destroy(lve_first_app* lvefirapp){
    lvewin_destroy(lvefirapp->m_window);
    free(lvefirapp);
}

void lvefirapp_run(lve_first_app* lvefirapp){
    while(!lvewin_shouldclose(lvefirapp->m_window)){
        glfwPollEvents();
    }
}