#include <lib.h>

int main(){
    message m;
    _syscall(VFS_PROC_NR, VFS_WHOSTHERE, &m);
    return 0;
}