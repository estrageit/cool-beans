#include "first_app.h"

int main(){
    lve_first_app* app = lvefirapp_make();

    lvefirapp_run(app);

    lvefirapp_destroy(app);
    //theres no try/catch in c :(
}