#pragma once

#define GLFW_INCLUDE_VULKAN
#include<GLFW/glfw3.h>

#include <stdint.h>

typedef struct lve_window{
    GLFWwindow* m_window;
    int m_width;
    int m_height;
    uint8_t m_frame_buffer_resized;
    char* m_name;
} lve_window;

lve_window* lvewin_make(int w, int h, const char* name);
void lvewin_destroy(lve_window* lvewin);
uint8_t lvewin_shouldclose(lve_window* lvewin);
uint8_t lvewin_was_window_resized(lve_window* lvewin);
void lvewin_reset_window_resized_flag(lve_window* lvewin);
void lvewin_create_window_surface(
    lve_window* lvewin,
    VkInstance instance,
    VkSurfaceKHR* surface);
VkExtent2D lvewin_get_extent(lve_window* lvewin);