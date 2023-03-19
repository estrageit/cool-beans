#pragma once

#include "lve_window.h"

typedef struct lve_swap_chain_support_details {
    VkSurfaceCapabilitiesKHR capabilities;
    VkSurfaceFormatKHR* formats;
    int formats_count;
    VkPresentModeKHR* present_modes;
    int present_modes_count;
} lve_swap_chain_support_details;

typedef struct lve_queue_family_indices {
  uint32_t graphics_family;
  uint32_t present_family;
  uint8_t graphics_family_has_value;// = false;
  uint8_t present_family_has_value;// = false;
} lve_queue_family_indices;

uint8_t lve_qfi_is_complete(lve_queue_family_indices* lve_qfi);

typedef struct lve_device{
    /*
    #ifdef NDEBUG
    const char enable_validation_layers = false;
    #else
    const bool enable_validation_layers = true;
    #endif
    */
    //public
    uint8_t enable_validation_layers;
    VkPhysicalDeviceProperties properties;

    //private
    VkInstance m_instance;
    VkDebugUtilsMessengerEXT m_debug_messenger;
    VkPhysicalDevice m_physical_device; // = VK_NULL_HANDLE;
    lve_window* m_window;
    VkCommandPool m_command_pool;

    VkDevice m_device;
    VkSurfaceKHR m_surface;
    VkQueue m_graphics_queue;
    VkQueue m_present_queue;

    char* m_validation_layers[1]; // = {"VK_LAYER_KHRONOS_validation"};
    int m_validation_layers_count;
    char* m_device_extensions[1]; // = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    int m_device_extensions_count;
} lve_device;

lve_device* lvedev_make(lve_window* window);
void lvedev_destroy(lve_device* lvedev);

//these are the public functions, private functions are in the .c file
VkCommandPool lvedev_get_command_pool(lve_device* lvedev);
VkDevice lvedev_device(lve_device* lvedev);
VkSurfaceKHR lvedev_surface(lve_device* lvedev);
VkQueue lvedev_graphics_queue(lve_device* lvedev);
VkQueue lvedev_present_queue(lve_device* lvedev);

lve_swap_chain_support_details lvedev_get_swap_chain_support(lve_device* lvedev);
uint32_t lvedev_find_memory_type(
    lve_device* lvedev, uint32_t type_filter, VkMemoryPropertyFlags properties);
lve_queue_family_indices lvedev_find_physical_queue_families(lve_device* lvedev);
VkFormat lvedev_find_supported_format(
    lve_device* lvedev,
    int candidates_count,
    const VkFormat* candidates,
    VkImageTiling tiling,
    VkFormatFeatureFlags features);

void lvedev_create_buffer(
    lve_device* lvedev,
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VkBuffer* buffer,
    VkDeviceMemory* buffer_memory);
VkCommandBuffer lvedev_begin_single_time_commands(lve_device* lvedev);
void lvedev_end_single_time_commands(
    lve_device* lvedev,
    VkCommandBuffer command_buffer);
void lvedev_end_single_time_commands(
    lve_device* lvedev,
    VkCommandBuffer command_buffer);
void lvedev_copy_buffer(
    lve_device* lvedev,
    VkBuffer src_buffer,
    VkBuffer dst_buffer,
    VkDeviceSize size);
void lvedev_copy_buffer_to_image(
    lve_device* lvedev,
    VkBuffer buffer,
    VkImage image,
    uint32_t width,
    uint32_t height,
    uint32_t layer_count);

void lvedev_create_image_with_info(
    lve_device* lvedev,
    const VkImageCreateInfo* image_info,
    VkMemoryPropertyFlags properties,
    VkImage* image,
    VkDeviceMemory* image_memory);