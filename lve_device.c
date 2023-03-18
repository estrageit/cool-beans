#include "lve_device.h"

lve_device* lvedev_make(lve_window* window){
    lve_device* r = malloc(sizeof(lve_device));
    r->m_window = window;



    return r;
}

VkDevice lvedev_device(lve_device* lvedev){
    
}