#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

int main(){
    if (setuid(0)){
        perror("setuid");
        return 1;
    }
    
}

