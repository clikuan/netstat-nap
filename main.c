#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <getopt.h>

int main(int argc, char *argv[]){

    if (setuid(0)){
        fprintf(stderr, "setuid error");
        return 1;
    }
    else{
        char * const short_options = "tu";
        const struct option long_options[] = {
            {  "tcp",      0,   NULL,   't'  },
            {  "udp",   0,   NULL,   'u'  },
            {  NULL,      0,    NULL,   0  }
        };
        int c;
        while((c = getopt_long (argc, argv, short_options, long_options, NULL)) != -1) {
            switch(c){
                case 't':
                    printf("tttt\n");
                    break;
                case 'u':
                    printf("uuuu\n");
                    break;
            }
        }
    }
    return 0;
}

