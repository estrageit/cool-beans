#include "lve_window.h"

#include <stdlib.h>
#include <string.h>

#include <stdio.h>

inline void lvewin_reset_window_resized_flag(lve_window* lvewin){
    lvewin->m_frame_buffer_resized = 0;
}

inline uint8_t lvewin_was_window_resized(lve_window* lvewin) {
    return lvewin->m_frame_buffer_resized;
}

void lvewin_framebuffer_resize_callback(GLFWwindow* win, int width, int height){
    lve_window* lvewin = (lve_window*)win;
    lvewin->m_frame_buffer_resized = 1;
    lvewin->m_width = width;
    lvewin->m_height = height;
}

void lvewin_init(lve_window* lvewin){
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    lvewin->m_window = glfwCreateWindow(
        lvewin->m_width,
        lvewin->m_height,
        lvewin->m_name,
        NULL,
        NULL);
    glfwSetWindowUserPointer(lvewin->m_window, lvewin);
    glfwSetFramebufferSizeCallback(lvewin->m_window, lvewin_framebuffer_resize_callback);
}

lve_window* lvewin_make(int w, int h, const char* name){
    lve_window* r = malloc(sizeof(lve_window));
    *r = (lve_window){};
    r->m_height = h;
    r->m_width = w;
    r->m_frame_buffer_resized = 0;
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

uint8_t lvewin_shouldclose(lve_window* lvewin) {
    return glfwWindowShouldClose(lvewin->m_window);
}

void lvewin_create_window_surface(
    lve_window* lvewin,
    VkInstance instance,
    VkSurfaceKHR* surface)
{
    if(glfwCreateWindowSurface(
        instance, lvewin->m_window, NULL, surface) != VK_SUCCESS)
        printf("\033[0;31m[ERROR]\033[0m glfwCreateWindowSurface failed!");
}

VkExtent2D lvewin_get_extent(lve_window* lvewin)
    {return (VkExtent2D){lvewin->m_width, lvewin->m_height};}