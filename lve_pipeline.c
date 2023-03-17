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

void lvepili_create_graphics_pipeline(
    const char* vert_file_path,
    const char* frag_file_path)
{
    lvepili_shader_buf* vert_code = lvepili_read_file(vert_file_path);
    printf("[INFO] Vert shader size: %d\n", vert_code->size);
    lvepili_shader_buf* frag_code = lvepili_read_file(frag_file_path);
    printf("[INFO] Frag shader size: %d\n", frag_code->size);

    free(vert_code);
    free(frag_code);
}

lve_pipeline* lvepili_make(
    const char* vert_file_path,
    const char* frag_file_path)
{
    lvepili_create_graphics_pipeline(vert_file_path, frag_file_path);
}

void lvepili_destroy(lve_pipeline* lvepili){

}