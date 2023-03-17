typedef struct lve_pipeline{
} lve_pipeline;

lve_pipeline* lvepili_make(
    const char* vert_file_path,
    const char* frag_file_path);
void lvepili_destroy(lve_pipeline* lvepili);