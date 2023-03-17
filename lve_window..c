#include "lve_window.h"

#include <stdlib.h>
#include <string.h>

#include <stdio.h>

void lvewin_init(lve_window* lvewin){
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    lvewin->m_window = glfwCreateWindow(
        lvewin->m_width,
        lvewin->m_height,
        lvewin->m_name,
        NULL,
        NULL);
}

lve_window* lvewin_make(int w, int h, const char* name){
    lve_window* r = malloc(sizeof(lve_window));
    r->m_height = h;
    r->m_width = w;
    char* n = malloc((strlen(name) + 1) * sizeof(char));
    strcpy(n, name);
    r->m_name = n;
    lvewin_init(r);
    return r;
}

void lvewin_destroy(lve_window* lvewin){
    glfwDestroyWindow(lvewin->m_window);
    glfwTerminate();
    free(lvewin->m_name);
    free(lvewin);
}

int lvewin_shouldclose(lve_window* lvewin) { 
    return glfwWindowShouldClose(lvewin->m_window); 
}