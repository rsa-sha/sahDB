#include "common.h"
#include "db.h"

void eventLoop(){
    err_t res = 0;
    while (res!=SIG_EXIT){
        res = getAndProcessCommand();
    }
    return;
}

int main(){
    eventLoop();
    return 0;
}