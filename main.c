#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>

void query();
void currentSocketPorcess();
void freeList();
int isDirectory(const char *);
char* readFile(char *);

typedef struct element{
    char protocol[5];
    char localAdderss[25];
    char foreignAddress[25];
    char pidNameArguments[256];
    struct element *next;
}LISTELEMENT;
LISTELEMENT *lHead = NULL;
LISTELEMENT *lTail = NULL;

typedef struct pElement{
    char pid[20]; 
    char pidNameArguments[256];
    char inodeID[20];
    struct pElement *next;
}PIDINFO;
PIDINFO *pHead = NULL;
PIDINFO *pTail = NULL;

int main(int argc, char *argv[]){

    if (setuid(0)){
        fprintf(stderr, "setuid error\n");
        return 1;
    }
    else{
        char * const short_options = "tu";
        const struct option long_options[] = {
            {"tcp",0,NULL,'t'},
            {"udp",0,NULL,'u'},
            {NULL,0,NULL,0 }
        };
        int c;
        while((c = getopt_long (argc, argv, short_options, long_options, NULL)) != -1) {
            switch(c){
                case 't':
                    currentSocketPorcess();
                    query(0, 0);
                    query(0, 1);
                    break;
                case 'u':
                    query(1, 0);
                    query(1, 1);
                    break;
            }
        }
    }
    freeList();LISTELEMENT *head = NULL;

    return 0;
}
void query(int TCPUDP, int version){
    FILE *fptr;
    char lineBuf[1000];
    if(!TCPUDP){
        if(!version)
            fptr = fopen("/proc/net/tcp","r");
        else
            fptr = fopen("/proc/net/tcp6","r");
    }
    else{
        if(!version)
            fptr = fopen("/proc/net/udp","r");
        else
            fptr = fopen("/proc/net/udp6","r");
    }
    LISTELEMENT *curr = NULL;
    fgets(lineBuf, 1000, fptr);//skip first line
    while(fgets(lineBuf, 1000, fptr) != NULL){
        curr = malloc(sizeof(LISTELEMENT));
        curr -> next = NULL;
        if(!TCPUDP){
            if(!version)
                strcpy(curr -> protocol,"tcp");
            else
                strcpy(curr -> protocol,"tcp6");
        }
        else{
            if(!version)
                strcpy(curr -> protocol,"udp");
            else
                strcpy(curr -> protocol,"udp6");
        }
        if(!lHead){
            lHead = curr;
            lTail = lHead;
        }
        else{
            lTail -> next = curr;
            lTail = lTail -> next;
        }
        int i = 0;
        char *token;
        char *delim = " ";
        token = strtok(lineBuf,delim);
        while (token != NULL){
            if(i == 1)
                strcpy(curr -> localAdderss, token);
            else if(i == 2)
                strcpy(curr -> foreignAddress, token);
            else if(i == 9){
                //printf("%s\n", token);
                //inode2Pid(colBuf);
                //strcpy(curr -> localAdderss, );
            }
            i++;
            token = strtok (NULL, delim);
        }
    }
    /*LISTELEMENT *i;
    for( i = lHead; i != NULL; i = i -> next)
        printf("%s\n", i->localAdderss);*/
}
void currentSocketPorcess(){
    struct dirent *procDirent;
    DIR *procDir;
    procDir = opendir("/proc");
    if(ENOENT == errno){
        fprintf(stderr, "/proc directory doesn't exist\n");
        closedir(procDir);
        exit(1);
    }
    char *pidNameArguments = NULL;
    while ((procDirent = readdir(procDir)) != NULL) {
        if(!(strcmp(procDirent->d_name,".") && strcmp(procDirent->d_name,"..")))
            continue;
        struct dirent *pidDirent;
        DIR *pidDir;
        char pidPath[256] = "/proc/";
        strcat(pidPath, procDirent -> d_name);
        strcat(pidPath, "/fd");
        if(!isDirectory(pidPath))
            continue;
        //printf("%s\n",pidPath);
        pidDir = opendir(pidPath);
        if(ENOENT == errno){
            closedir(pidDir);
            continue;
        }
        if(pidNameArguments != NULL){
            //char *t = pidNameArguments;
            free(pidNameArguments);
            //pidNameArguments = NULL;
        }
        while((pidDirent = readdir(pidDir)) != NULL){
            if(!(strcmp(pidDirent->d_name,".") && strcmp(pidDirent->d_name,"..")))
                continue;
            char linkBuf[256];
            char fdPath[256];
            memset(linkBuf, 0, 256);
            memset(fdPath, 0, 256);
            strcpy(fdPath, pidPath);
            strcat(fdPath, "/");
            strcat(fdPath, pidDirent->d_name);
            if(readlink(fdPath, linkBuf, 255) > 0){
                char *token;
                char *delim = ":";
                token = strtok(linkBuf, delim);
                PIDINFO *curr;
                if (!strcmp(token, "socket")){
                    token = strtok(NULL, delim);
                    token[strlen(token)-1] = '\0';
                    token++;
                    curr = malloc(sizeof(PIDINFO));
                    curr -> next = NULL;
                    strcpy(curr -> pid, procDirent -> d_name);
                    strcpy(curr -> inodeID, token);
                    if(pidNameArguments == NULL){
                        char path[256] = "/proc/";
                        strcat(path ,curr -> pid);
                        strcat(path, "/cmdline");
                        readFile(path);
                        //printf("%s\n", pidNameArguments);
                    }
                    //strcpy(curr -> pidNameArguments, pidNameArguments);
                    if(!pHead){
                        pHead = curr;
                        pTail = pHead;
                    }
                    else{
                        pTail -> next = curr;
                        pTail = pTail -> next;
                    }
                    //printf("%s\n", curr -> pidNameArguments);
                }
                
                //printf("less /proc/%s/cmdline\n", curr -> pid);
                //printf("[%s]->%s\n", fdPath, linkBuf);
            }
            //printf ("[%s]\n", pidDirent->d_name);
        }
        closedir(pidDir);
    }
    closedir(procDir);
}
int isDirectory(const char *path) {
   struct stat statbuf;
   if (stat(path, &statbuf) != 0)
       return 0;
   return S_ISDIR(statbuf.st_mode);
}
char* readFile(char *path){
   int string_size, read_size;
   char *buffer;
   FILE *handler = fopen(path, "r");
   if (handler)
   {
       // Seek the last byte of the file
       fseek(handler, 0, SEEK_END);
       // Offset from the first to the last byte, or in other words, filesize
       string_size = ftell(handler);
       // go back to the start of the file
       rewind(handler);
       
       // Allocate a string that can hold it all
       buffer = (char*) malloc(sizeof(char) * (string_size + 1) );
       // Read it all in one operation
       read_size = fread(buffer, sizeof(char), string_size, handler);
       // fread doesn't set it so put a \0 in the last position
       // and buffer is now officially a string
       (buffer)[string_size] = '\0';

       if (string_size != read_size)
       {
           // Something went wrong, throw away the memory and set
           // the buffer to NULL
           free(buffer);
           buffer = NULL;
       }
       // Always remember to close the file.
       fclose(handler);
    }
    printf("%s\n",buffer);
    return buffer;
}
void freeList(){
    LISTELEMENT *i;
    for( i = lHead; i != NULL;){
        LISTELEMENT *t = i;
        free(t);
        i = i -> next;
    }
}
