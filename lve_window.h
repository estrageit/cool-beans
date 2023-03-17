#pragma once

#define GLFW_INCLUDE_VULKAN
#include<GLFW/glfw3.h>

#include <stdint.h>

typedef struct lve_window{
    GLFWwindow* m_window;
    int m_width;
    int m_height;
    char* m_name;
} lve_window;

lve_window* lvewin_make(int w, int h, const char* name);
void lvewin_destroy(lve_window* lvewin);
uint8_t lvewin_shouldclose(lve_window* lvewin);