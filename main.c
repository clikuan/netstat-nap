#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>

void query();
char queryFile[4][100] = {
        "/proc/net/tcp",
        "/proc/net/tcp6",
        "/proc/net/udp",
        "/proc/net/udp6"
};
typedef struct element{
    char protocol[5];
    char localAdderss[25];
    char foreignAddress[25];
    char pidNameArguments[256];
    struct element *next;
}LISTELEMENT;
LISTELEMENT *head = NULL;

int main(int argc, char *argv[]){

//    if (setuid(0)){
//        fprintf(stderr, "setuid error\n");
//        return 1;
//    }
//    else{
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
                    query(0);
                    break;
                case 'u':
                    query(1);
                    break;
            }
        }
    //}
    return 0;
}
void query(int TCPUDP){
    FILE *fptr1, *fptr2;
    char lineBuf[1000];
    char colBuf[256];
    if(!TCPUDP){
        fptr1 = fopen("/proc/net/tcp","r");
        fptr2 = fopen("/proc/net/tcp6","r");
    }
    else{
        fptr1 = fopen("/proc/net/udp","r");
        fptr2 = fopen("/proc/net/udp6","r");
    }
    LISTELEMENT *curr = NULL;
    LISTELEMENT *tail = NULL
    fgets(lineBuf, 1000, fptr1);//skip first line
    while(fgets(lineBuf, 1000, fptr1) != NULL){
        curr = malloc(sizeof(LISTELEMENT));
        curr -> next = NULL;
        if(!TCPUDP)
            strcpy(curr -> protocol,"tcp");
        if(!head){
            head = curr;
            tail = head;
        }
        else{
            tail -> next = curr;
            tail = tail -> next;
        }
        int i = 0;
        while(sscanf(lineBuf, "%s", colBuf) != EOF){
            printf("%s", colBuf);
            if(i == 1)
                strcpy(curr -> localAdderss, colBuf);
            else if(i == 2)
                strcpy(curr -> foreignAddress, colBuf);
            else if(i == 9){
                //inode2Pid(colBuf);
                //strcpy(curr -> localAdderss, );
            }
            i++;
        }
    }
}
