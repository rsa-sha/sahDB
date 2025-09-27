#include "common.h"
#include "db.h"
#include "hash.h"

void eventLoop(){
    err_t res = 0;
    while (res!=SIG_EXIT){
        res = getAndProcessCommand();
    }
    return;
}

int main(){
    ht_init();
    eventLoop();
    return 0;
}