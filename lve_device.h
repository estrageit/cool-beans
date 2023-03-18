#pragma once

#include "lve_window.h"

typedef struct lve_device{
    lve_window* m_window;
} lve_device;

lve_device* lvedev_make(lve_window* window);
void lvedev_destroy(lve_device* lvedev);

VkDevice lvedev_device(lve_device* lvedev);