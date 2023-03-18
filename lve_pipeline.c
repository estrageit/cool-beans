#include "lve_pipeline.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef struct lvepili_shader_buf{
    int size;
    uint8_t buf[];
} lvepili_shader_buf;

// IMPORTANT: the return buffer for this HAS to be freed
lvepili_shader_buf* lvepili_read_file(const char* file_path){
    FILE *fp = fopen(file_path, "rb");
    if (fp == NULL) {
        printf("[ERROR] Could not read file '%s'. ferror() returned the following: (%0x)\n", file_path, ferror(fp));
        return NULL;
    }
    fseek(fp, 0L, SEEK_END);
    int sz = ftell(fp);
    rewind(fp);
    lvepili_shader_buf* r = malloc(sizeof(int) + sz);
    r->size = sz;
    fgets(r->buf, sz, fp);
    fclose(fp);
    return r;
}

void lvepili_create_shader_module(lve_pipeline* lvepili, const lvepili_shader_buf* code, VkShaderModule* shader_module){
    VkShaderModuleCreateInfo create_info;
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = code->size; // could need a +1
    create_info.pCode = (const uint32_t*)code->buf; // there's an incredibly high chance this doesnt work
    if (vkCreateShaderModule(lvedev_device(lvepili->m_device), &create_info, NULL, shader_module) != VK_SUCCESS)
        printf("[ERROR] lvepili_create_shader_module() failed!\n");
}

void lvepili_create_graphics_pipeline(
    const char* vert_file_path,
    const char* frag_file_path,
    const lve_pipeline_config_info* config_info)
{
    lvepili_shader_buf* vert_code = lvepili_read_file(vert_file_path);
    printf("[INFO] Vert shader size: %d\n", vert_code->size);
    lvepili_shader_buf* frag_code = lvepili_read_file(frag_file_path);
    printf("[INFO] Frag shader size: %d\n", frag_code->size);

    free(vert_code);
    free(frag_code);
}

lve_pipeline* lvepili_make(
    lve_device* device,
    const char* vert_file_path,
    const char* frag_file_path,
    const lve_pipeline_config_info* config_info)
{
    lve_pipeline* r = malloc(sizeof(lve_pipeline));

    r->m_device = device;

    lvepili_create_graphics_pipeline(vert_file_path, frag_file_path, config_info);
    
    return r;
}

void lvepili_destroy(lve_pipeline* lvepili){

}

lve_pipeline_config_info* lvepili_default_pipeline_config_info(
    uint32_t width, uint32_t height)
{
    lve_pipeline_config_info* r = malloc(sizeof(lve_pipeline_config_info));
    return r;
}

//theres a chance that this isnt working if something glitches
//out try checking here for errors