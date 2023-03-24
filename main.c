#include "first_app.h"

int main(){
    first_app* app = firapp_make();
 
    firapp_run(app);

    firapp_destroy(app);
    //theres no try/catch in c :(
}