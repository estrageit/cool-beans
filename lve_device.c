#include "lve_device.h"

#include<stdlib.h>
#include<string.h>
#include<stdio.h>

// its VERY possible that none of this actually works

uint8_t lve_qfi_is_complete(lve_queue_family_indices* lve_qfi){
    return lve_qfi->graphics_family_has_value && lve_qfi->present_family_has_value;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData) {
    printf("\033[0;32m[VALIDATION_LAYER]\033[0m %s\n", pCallbackData->pMessage);
    return VK_FALSE;
}
VkResult create_debug_utils_messenger_ext(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkDebugUtilsMessengerEXT *pDebugMessenger) {
    PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
        instance,
        "vkCreateDebugUtilsMessengerEXT");
    if (func != NULL) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void destroy_debug_utils_messenger_ext(
    VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks *pAllocator) {
    PFN_vkDestroyDebugUtilsMessengerEXT auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
        instance,
        "vkDestroyDebugUtilsMessengerEXT");
    if (func != NULL) {
        func(instance, debugMessenger, pAllocator);
    }
}

typedef struct lve_extension_vector{
    int count;
    char* extensions[];
} lve_extension_vector;

//helper functions
lve_extension_vector* lvedev_get_required_extensions(lve_device* lvedev){
    uint32_t temp_glfw_extensions_count = 0;
    const char **temp_glfw_extensions;
    temp_glfw_extensions = glfwGetRequiredInstanceExtensions(&temp_glfw_extensions_count);
    int added = 0;
    if (lvedev->enable_validation_layers) {
        added = 1;
    }
    lve_extension_vector* r = malloc(sizeof(lve_extension_vector) + ((added + temp_glfw_extensions_count) * sizeof(char*)));
    r->count = added + temp_glfw_extensions_count;
    memcpy(r->extensions, temp_glfw_extensions, sizeof(char*) * temp_glfw_extensions_count);
    if (lvedev->enable_validation_layers) {
        r->extensions[temp_glfw_extensions_count] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
    }
    return r;
}

lve_queue_family_indices lvedev_find_queue_families(lve_device* lvedev, VkPhysicalDevice device){
    lve_queue_family_indices indices;
    indices.graphics_family_has_value = 0;
    indices.present_family_has_value = 0;
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, NULL);
    VkQueueFamilyProperties* queue_families = malloc(sizeof(VkQueueFamilyProperties) * queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queue_families);
    for(int i=0; i<queueFamilyCount; i++){
        if (queue_families[i].queueCount > 0 && (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
            indices.graphics_family = i;
            indices.graphics_family_has_value = 1;
        }
        VkBool32 presentSupport = 0;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, lvedev->m_surface, &presentSupport);
        if (queue_families[i].queueCount > 0 && presentSupport) {
            indices.present_family = i;
            indices.present_family_has_value = 1;
        }
        if (lve_qfi_is_complete(&indices)) {
            break;
        }
    }
    return indices;
}

lve_swap_chain_support_details lvedev_query_swap_chain_support(lve_device* lvedev, VkPhysicalDevice device){
    lve_swap_chain_support_details details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, lvedev->m_surface, &details.capabilities);
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, lvedev->m_surface, &formatCount, NULL);
    if (formatCount != 0) {
        details.formats = malloc(formatCount * sizeof(VkSurfaceFormatKHR));
        details.formats_count = formatCount; //might be the size instead of count
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, lvedev->m_surface, &formatCount, details.formats);
    }
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, lvedev->m_surface, &presentModeCount, NULL);
    if (presentModeCount != 0) {
        details.present_modes = malloc(presentModeCount * sizeof(VkPresentModeKHR));
        details.present_modes_count = presentModeCount; //might be the size instead of count
        vkGetPhysicalDeviceSurfacePresentModesKHR(
            device,
            lvedev->m_surface,
            &presentModeCount,
            details.present_modes);
    }
    return details;
}

//this is SUPPOSED to return a 1 always
uint8_t lvedev_check_device_extension_support(lve_device* lvedev, VkPhysicalDevice device){
    uint32_t extension_count;
    vkEnumerateDeviceExtensionProperties(device, NULL, &extension_count, NULL);
    VkExtensionProperties* available_extensions =
        malloc(sizeof(VkExtensionProperties) * extension_count);
    vkEnumerateDeviceExtensionProperties(
        device,
        NULL,
        &extension_count,
        available_extensions);
    //i'm going to ASSUME that what this is supposed to do is check if
    //available_extensions has all of device_extensions
    for(int i=0; i<lvedev->m_device_extensions_count; i++){
        int r = 1;
        for(int j=0; j<extension_count; j++){
            if(strcmp(lvedev->m_device_extensions[i], available_extensions[j].extensionName) == 0){
                r = 0;
                break;
            }
        }
        if(r){
            return 0;
        }
    }
    return 1;
}

uint8_t lvedev_is_device_suitable(lve_device* lvedev, VkPhysicalDevice device){
    lve_queue_family_indices indices = lvedev_find_queue_families(lvedev, device);
    uint8_t extensions_supported = lvedev_check_device_extension_support(lvedev, device);
    uint8_t swap_chain_adequate = 0;
    if (extensions_supported) {
        lve_swap_chain_support_details swapChainSupport = lvedev_query_swap_chain_support(lvedev, device);
        swap_chain_adequate = !(swapChainSupport.formats_count == 0) && !(swapChainSupport.present_modes_count == 0);
    }
    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(device, &supportedFeatures);
    return lve_qfi_is_complete(&indices) && extensions_supported && swap_chain_adequate &&
        supportedFeatures.samplerAnisotropy;
}

uint8_t lvedev_check_validation_layer_support(lve_device* lvedev){
    uint32_t layer_count;
    vkEnumerateInstanceLayerProperties(&layer_count, NULL);
    VkLayerProperties* available_layers = malloc(sizeof(VkLayerProperties) * layer_count);
    vkEnumerateInstanceLayerProperties(&layer_count, available_layers);
    for(int i=0;i<lvedev->m_validation_layers_count;i++){
        uint8_t found = 0;
        for(int j=0;j<layer_count;j++){
            if (strcmp(lvedev->m_validation_layers[i], available_layers[j].layerName) == 0) {
                found = 1;
                break;
            }
        }
        if(!found)
            return 0;
    }
    return 1;
}

void lvedev_populate_debug_messenger_create_info(
    lve_device* lvedev, VkDebugUtilsMessengerCreateInfoEXT* create_info){
    *create_info = (VkDebugUtilsMessengerCreateInfoEXT){};
    create_info->sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    create_info->messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                                VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;// |
                                //VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
    create_info->messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    create_info->pfnUserCallback = debug_callback;
    create_info->pUserData = NULL;  // Optional
}

void lvedev_has_gflw_required_instance_extensions(lve_device* lvedev){
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);
    VkExtensionProperties* extensions = malloc(sizeof(VkExtensionProperties) * extensionCount);
    vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, extensions);
    printf("\033[0;34m[INFO]\033[0m Available extensions:\n");
    for(int i=0;i<extensionCount;i++){
        printf("\t%s\n",extensions[i].extensionName);
    }
    printf("\033[0;34m[INFO]\033[0m Required extensions:\n");
    lve_extension_vector* required = lvedev_get_required_extensions(lvedev);
    for(int i=0;i<required->count;i++){
        printf("\t%s\n",required->extensions[i]);
        int found=0;
        for(int j=0;j<extensionCount;j++){
            if(strcmp(required->extensions[i], extensions[j].extensionName)==0){
                found=1;
                break;
            }
        }
        if(!found){
            printf("\033[0;31m[ERROR]\033[0m Missing required glfw extension '%s'!\n", required->extensions[i]);
            exit(-1);
        }
    }
}

//private functions
void lvedev_create_instance(lve_device* lvedev){
    if (lvedev->enable_validation_layers && !lvedev_check_validation_layer_support(lvedev)){
        printf("\033[0;31m[ERROR]\033[0m Validation layers requested, but not available!\n");
        exit(-1);
    }
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "LittleVulkanEngine App";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;
    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    lve_extension_vector* extensions = lvedev_get_required_extensions(lvedev);
    createInfo.enabledExtensionCount = extensions->count;
    createInfo.ppEnabledExtensionNames = (const char* const*)extensions->extensions;
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
    if (lvedev->enable_validation_layers) {
        createInfo.enabledLayerCount = lvedev->m_validation_layers_count;
        createInfo.ppEnabledLayerNames = (const char* const*)lvedev->m_validation_layers;
        lvedev_populate_debug_messenger_create_info(lvedev, &debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
    } else {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = NULL;
    }
    if (vkCreateInstance(&createInfo, NULL, &lvedev->m_instance) != VK_SUCCESS) {
        printf("\033[0;31m[ERROR]\033[0m Failed to create instance!\n");
        exit(-1);
    }
    lvedev_has_gflw_required_instance_extensions(lvedev);
}

void lvedev_setup_debug_messenger(lve_device* lvedev){
    if (!lvedev->enable_validation_layers) return;
    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    lvedev_populate_debug_messenger_create_info(lvedev, &createInfo);
    if (create_debug_utils_messenger_ext(lvedev->m_instance, &createInfo, NULL, &lvedev->m_debug_messenger) != VK_SUCCESS) {
        printf("\033[0;31m[ERROR]\033[0m Failed to set up debug messenger!\n");
        exit(-1);
    }
}

void lvedev_create_surface(lve_device* lvedev){
    lvewin_create_window_surface(lvedev->m_window, lvedev->m_instance, &lvedev->m_surface);
}

void lvedev_pick_physical_device(lve_device* lvedev){
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(lvedev->m_instance, &deviceCount, NULL);
    if (deviceCount == 0) {
        printf("\033[0;31m[ERROR]\033[0m Failed to find GPUs with Vulkan support!\n");
        exit(-1);
    }
    printf("\033[0;34m[INFO]\033[0m Device count: %d\n", deviceCount);
    VkPhysicalDevice* devices = malloc(sizeof(VkPhysicalDevice) * deviceCount);
    vkEnumeratePhysicalDevices(lvedev->m_instance, &deviceCount, devices);
    for(int i=0;i<deviceCount;i++){
        if(lvedev_is_device_suitable(lvedev, devices[i])){
            lvedev->m_physical_device = devices[i];
            break;
        }
    }
    if(lvedev->m_physical_device == VK_NULL_HANDLE){
        printf("\033[0;31m[ERROR]\033[0m Failed to find GPUs with Vulkan support!\n");
        exit(-1);
    }
    vkGetPhysicalDeviceProperties(lvedev->m_physical_device, &lvedev->properties);
    printf("\033[0;34m[INFO]\033[0m Physical device: '%s'\n", lvedev->properties.deviceName);
}

//FIX THIS LATER SK8ER
void lvedev_create_logical_device(lve_device* lvedev){
    lve_queue_family_indices indices = lvedev_find_queue_families(lvedev, lvedev->m_physical_device);
    VkDeviceQueueCreateInfo* queue_create_infos = malloc(sizeof(VkDeviceQueueCreateInfo) * 1); // * 2
    float queue_priority = 1.0f;
    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = indices.graphics_family;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queue_priority;
    queue_create_infos[0] = queueCreateInfo;
    /*
    queueCreateInfo = (VkDeviceQueueCreateInfo){};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = indices.present_family;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queue_priority;
    queue_create_infos[1] = queueCreateInfo;
    */
    VkPhysicalDeviceFeatures deviceFeatures = {};
    deviceFeatures.samplerAnisotropy = VK_TRUE;
    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = 1; //????? 2
    createInfo.pQueueCreateInfos = queue_create_infos;
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = lvedev->m_device_extensions_count;
    createInfo.ppEnabledExtensionNames = (const char* const*)lvedev->m_device_extensions;
    // might not really be necessary anymore because device specific validation layers
    // have been deprecated
    if (lvedev->enable_validation_layers) {
        createInfo.enabledLayerCount = lvedev->m_validation_layers_count;
        createInfo.ppEnabledLayerNames = (const char* const*)lvedev->m_validation_layers;
    } else {
        createInfo.enabledLayerCount = 0;
    }
    if (vkCreateDevice(lvedev->m_physical_device, &createInfo, NULL, &lvedev->m_device)
        != VK_SUCCESS) {
        printf("\033[0;31m[ERROR]\033[0m Failed to create logical device!\n");
        exit(-1);
    }

    vkGetDeviceQueue(lvedev->m_device, indices.graphics_family, 0, &lvedev->m_graphics_queue);
    vkGetDeviceQueue(lvedev->m_device, indices.present_family, 0, &lvedev->m_present_queue);
}

void lvedev_create_command_pool(lve_device* lvedev){
    lve_queue_family_indices queueFamilyIndices = lvedev_find_physical_queue_families(lvedev);
    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphics_family;
    poolInfo.flags =
        VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    if(vkCreateCommandPool(lvedev->m_device, &poolInfo, NULL, &lvedev->m_command_pool)){
        printf("\033[0;31m[ERROR]\033[0m Failed to create command pool!\n");
        exit(-1);
    }
}

//below is public function implementations
lve_device* lvedev_make(lve_window* window){
    lve_device* r = malloc(sizeof(lve_device));
    r->m_window = window;
    #ifdef NDEBUG
    r->enable_validation_layers = 0;
    #else
    r->enable_validation_layers = 1;
    #endif
    r->m_physical_device = VK_NULL_HANDLE;
    r->m_validation_layers[0] = "VK_LAYER_KHRONOS_validation";
    r->m_validation_layers_count = 1;
    r->m_device_extensions[0] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
    r->m_device_extensions_count = 1;

    lvedev_create_instance(r);
    lvedev_setup_debug_messenger(r);
    lvedev_create_surface(r);
    lvedev_pick_physical_device(r);
    lvedev_create_logical_device(r);
    lvedev_create_command_pool(r);

    return r;
}

void lvedev_destroy(lve_device* lvedev){
  vkDestroyCommandPool(lvedev->m_device, lvedev->m_command_pool, NULL);
  vkDestroyDevice(lvedev->m_device, NULL);
  if (lvedev->enable_validation_layers) {
    destroy_debug_utils_messenger_ext(lvedev->m_instance, lvedev->m_debug_messenger, NULL);
  }
  vkDestroySurfaceKHR(lvedev->m_instance, lvedev->m_surface, NULL);
  vkDestroyInstance(lvedev->m_instance, NULL);
  free(lvedev);
}

VkCommandPool lvedev_get_command_pool(lve_device* lvedev){return lvedev->m_command_pool;}
VkDevice lvedev_device(lve_device* lvedev){return lvedev->m_device;}
VkSurfaceKHR lvedev_surface(lve_device* lvedev){return lvedev->m_surface;}
VkQueue lvedev_graphics_queue(lve_device* lvedev){return lvedev->m_graphics_queue;}
VkQueue lvedev_present_queue(lve_device* lvedev){return lvedev->m_present_queue;}

lve_swap_chain_support_details lvedev_get_swap_chain_support(lve_device* lvedev)
    {return lvedev_query_swap_chain_support(lvedev, lvedev->m_physical_device);}

uint32_t lvedev_find_memory_type(
    lve_device* lvedev, uint32_t type_filter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(lvedev->m_physical_device, &memProperties);
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((type_filter & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
        return i;
        }
    }
    printf("\033[0;31m[ERROR]\033[0m Failed to find suitable memory type!\n");
    exit(-1);
}

lve_queue_family_indices lvedev_find_physical_queue_families(lve_device* lvedev)
    {return lvedev_find_queue_families(lvedev, lvedev->m_physical_device);}

VkFormat lvedev_find_supported_format(
    lve_device* lvedev,
    int candidates_count,
    const VkFormat* candidates,
    VkImageTiling tiling,
    VkFormatFeatureFlags features)
{
    for(int i=0;i<candidates_count;i++){
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(lvedev->m_physical_device, candidates[i], &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
        return candidates[i];
        } else if (
            tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
        return candidates[i];
        }
    }
    printf("\033[0;31m[ERROR]\033[0m Failed to find supported format!\n");
    exit(-1);
}

void lvedev_create_buffer(
    lve_device* lvedev,
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VkBuffer* buffer,
    VkDeviceMemory* buffer_memory)
{
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    if (vkCreateBuffer(lvedev->m_device, &bufferInfo, NULL, buffer) != VK_SUCCESS) {
        printf("\033[0;31m[ERROR]\033[0m Failed to create vertex buffer!\n");
        exit(-1);
    }
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(lvedev->m_device, *buffer, &memRequirements);
    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = lvedev_find_memory_type(lvedev, memRequirements.memoryTypeBits, properties);
    if (vkAllocateMemory(lvedev->m_device, &allocInfo, NULL, buffer_memory) != VK_SUCCESS) {
        printf("\033[0;31m[ERROR]\033[0m Failed to allocate vertex buffer memory!\n");
        exit(-1);
    }
    vkBindBufferMemory(lvedev->m_device, *buffer, *buffer_memory, 0);
}

VkCommandBuffer lvedev_begin_single_time_commands(lve_device* lvedev)
{
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = lvedev->m_command_pool;
    allocInfo.commandBufferCount = 1;
    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(lvedev->m_device, &allocInfo, &commandBuffer);
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    return commandBuffer;
}

void lvedev_end_single_time_commands(
    lve_device* lvedev,
    VkCommandBuffer command_buffer)
{
  vkEndCommandBuffer(command_buffer);
  VkSubmitInfo submitInfo = {};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &command_buffer;
  vkQueueSubmit(lvedev->m_graphics_queue, 1, &submitInfo, VK_NULL_HANDLE);
  vkQueueWaitIdle(lvedev->m_graphics_queue);
  vkFreeCommandBuffers(lvedev->m_device, lvedev->m_command_pool, 1, &command_buffer);
}

void lvedev_copy_buffer(
    lve_device* lvedev,
    VkBuffer src_buffer,
    VkBuffer dst_buffer,
    VkDeviceSize size)
{
    VkCommandBuffer commandBuffer = lvedev_begin_single_time_commands(lvedev);
    VkBufferCopy copyRegion = {};
    copyRegion.srcOffset = 0;  // Optional
    copyRegion.dstOffset = 0;  // Optional
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, src_buffer, dst_buffer, 1, &copyRegion);
    lvedev_end_single_time_commands(lvedev, commandBuffer);
}

void lvedev_copy_buffer_to_image(
    lve_device* lvedev,
    VkBuffer buffer,
    VkImage image,
    uint32_t width,
    uint32_t height,
    uint32_t layer_count)
{
    VkCommandBuffer commandBuffer = lvedev_begin_single_time_commands(lvedev);
    VkBufferImageCopy region = {};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = layer_count;
    region.imageOffset = (VkOffset3D){0, 0, 0};
    region.imageExtent = (VkExtent3D){width, height, 1};
    vkCmdCopyBufferToImage(
        commandBuffer,
        buffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region);
    lvedev_end_single_time_commands(lvedev, commandBuffer);
}

void lvedev_create_image_with_info(
    lve_device* lvedev,
    const VkImageCreateInfo* image_info,
    VkMemoryPropertyFlags properties,
    VkImage* image,
    VkDeviceMemory* image_memory)
{
    if (vkCreateImage(lvedev->m_device, image_info, NULL, image) != VK_SUCCESS) {
        printf("\033[0;31m[ERROR]\033[0m Failed to create image!\n");
        exit(-1);
    }
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(lvedev->m_device, *image, &memRequirements);
    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = lvedev_find_memory_type(lvedev, memRequirements.memoryTypeBits, properties);
    if (vkAllocateMemory(lvedev->m_device, &allocInfo, NULL, image_memory) != VK_SUCCESS) {
        printf("\033[0;31m[ERROR]\033[0m Failed to allocate image memory!\n");
        exit(-1);
    }
    if (vkBindImageMemory(lvedev->m_device, *image, *image_memory, 0) != VK_SUCCESS) {
        printf("\033[0;31m[ERROR]\033[0m Failed to bind image memory!\n");
        exit(-1);
    }
}