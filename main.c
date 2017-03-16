#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <regex.h> 
#include <netinet/in.h>
#include <arpa/inet.h>

void query();
int vaildRegex(char *);
int stringMatch(char *, char *);
void currentSocketPorcess();
void freeList();
int isDirectory(const char *);
void readFile(char *, char *);
void printList(char *);
void binaryString2IP(int, char *, char *);

typedef struct element{
    char protocol[6];
    char localAddress[65];
    char foreignAddress[65];
    char pidNameArguments[256];
    struct element *next;
}LISTELEMENT;
LISTELEMENT *lHead = NULL;
LISTELEMENT *lTail = NULL;

typedef struct pElement{
    char pid[20]; 
    char nameArguments[256];
    char inodeID[20];
    struct pElement *next;
}PIDINFO;
PIDINFO *pHead = NULL;
PIDINFO *pTail = NULL;

int main(int argc, char *argv[]){
    int printFlag = 0;
    char *stringFiler = NULL;
    if (setuid(0)){
        fprintf(stderr, "setuid error\n");
        return 1;
    }
    else{
        currentSocketPorcess();
        char * const short_options = "tu";
        const struct option long_options[] = {
            {"tcp", 0, NULL,'t'},
            {"udp", 0, NULL,'u'},
            {NULL, 0, NULL, 0}
        };
        int c;
        int tcp = 0,udp = 0;
        while((c = getopt_long (argc, argv, short_options, long_options, NULL)) != -1) {
            switch(c){
                case 't':
                    tcp = 1;
                    break;
                case 'u':
                    udp = 1;
                    break;
                case '?':
                    fprintf(stderr, "command should be [-t|--tcp] [-u|--udp] [filter-string]\n");
                    exit(1);
                    break;
            }
        }
        argc -= optind;
        argv += optind;
        if(argc == 1){
            stringFiler = argv[0];
            if(!vaildRegex(stringFiler)){
                fprintf(stderr, "invaild regular expression");
                exit(1);
            }
        }
        else if(argc > 1){
            fprintf(stderr, "command should be [-t|--tcp] [-u|--udp] [filter-string]\n");
            exit(1);
        }
        if(optind == 1){
        	query(1, 0);
            query(1, 1);
            query(0, 0);
            query(0, 1);
        }
        if(tcp){
            query(1, 0);
            query(1, 1);
        }
        if(udp){
            query(0, 0);
            query(0, 1);
        }
    }
    printList(stringFiler);
    freeList();
    return 0;
}
void query(int TCP, int version){
    FILE *fptr;
    char lineBuf[1000];
    if(TCP){
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
        memset(curr,0,sizeof(LISTELEMENT));
        if(TCP){
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
            	strcpy(curr -> localAddress, token);
            else if(i == 2)
            	strcpy(curr -> foreignAddress, token);
            else if(i == 9){
                PIDINFO *i;
                for( i = pHead; i != NULL; i = i -> next){
                    if(!strcmp(i -> inodeID, token)){
                        strcpy(curr -> pidNameArguments, i -> pid);
                        strcat(curr -> pidNameArguments, "/");                       
                        char *name;
                        char *n;
                        char *d = "/";
                        name = strtok( i -> nameArguments, d);
                        while (name != NULL){
                            n = name;
                            name = strtok (NULL, d);  
                        }
                        strcat(curr -> pidNameArguments, n);
                        break;
                    }
                }
            }
            i++;
            token = strtok (NULL, delim);
        }
    }
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
        //printf("%s\n",pidPath);char
        pidDir = opendir(pidPath);
        if(ENOENT == errno){
            closedir(pidDir);
            continue;
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
                    char path[256] = "/proc/";
                    strcat(path ,curr -> pid);
                    strcat(path, "/cmdline");
                    readFile(path, curr -> nameArguments);
                    if(!pHead){
                        pHead = curr;
                        pTail = pHead;
                    }
                    else{
                        pTail -> next = curr;
                        pTail = pTail -> next;
                    }
                }
            }
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
void readFile(char *path, char *buffer){
    FILE *fptr = fopen(path, "r");
    int i = 0;
    char c;
    while((c = getc(fptr)) != EOF){
        buffer[i]= (c == '\0') ? ' ' : c;
        if(i == 254)
            break;
        i++;
    }
    buffer[i] = '\0';
    fclose(fptr);
}
int vaildRegex(char *r){
    regex_t regex;
    int reti = regcomp(&regex, r, 0);
    regfree(&regex);
    return !reti;
}
int stringMatch(char *r, char *s){
    regex_t regex;
    regcomp(&regex, r, 0);
    int reti = regexec(&regex, s, 0, NULL, 0);
    regfree(&regex);
    if (!reti) 
        return 1;
    else if (reti == REG_NOMATCH)
        return 0;
    
}
void printList(char *stringFiler){
    LISTELEMENT *t = malloc(sizeof(LISTELEMENT));
    strcpy(t -> protocol, "Proto");
    strcpy(t -> localAddress, "LocalAddress");
    strcpy(t -> foreignAddress, "ForeignAddress");
    strcpy(t -> pidNameArguments, "PID/Program name and arguments");
    t -> next = lHead;
    lHead = t;
    LISTELEMENT *i; 
    for( i = lHead; i != NULL; i = i -> next){
        if(i == lHead){
            printf("%-10s %-45s %-45s %-10s\n", i -> protocol, 
            i -> localAddress, i -> foreignAddress, i -> pidNameArguments);
            continue;
        }
        char local[100];
		char fore[100];
	    if(strcmp(i -> protocol, "tcp") == 0 || strcmp(i -> protocol, "udp") == 0){
	    	binaryString2IP(1, i -> localAddress, local);
			binaryString2IP(1, i -> foreignAddress, fore);
	    }
	    else{
	    	binaryString2IP(0, i -> localAddress, local);
	    	binaryString2IP(0, i -> foreignAddress, fore);

	    }
        if(stringFiler){
            if(stringMatch(stringFiler, i -> protocol) || stringMatch(stringFiler, i -> localAddress) 
                || stringMatch(stringFiler, i -> foreignAddress) || stringMatch(stringFiler, i -> pidNameArguments)){
                    printf("%-10s %-45s %-45s %-10s\n", i -> protocol, 
                        local, fore, i -> pidNameArguments);
            }
        }
        else{
            printf("%-10s %-45s %-45s %-10s\n", i -> protocol, 
                    local, fore, i -> pidNameArguments);
        }
     }
}
void binaryString2IP(int ipv4, char *binaryString, char *address){
    if(ipv4){
        char *ptr;
        char *d = ":";
        char *ip = strtok(binaryString, d);
        char *port = strtok(NULL, d);
        long p = strtol(port, &ptr, 16);
        struct in_addr addr;
        addr.s_addr = strtol(ip, &ptr, 16);
        inet_ntop(AF_INET, &(addr.s_addr), address, INET_ADDRSTRLEN);
        char p1[15];
        sprintf(p1, ":%ld", p);
        strcat(address, p1);
    }
    else{
    	struct in6_addr tmp_ip;
	char *ptr;
        char *d = ":";
        char *ip = strtok(binaryString, d);
        char *port = strtok(NULL, d);
		if (sscanf(ip,
		    "%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx",
		    &tmp_ip.s6_addr[3], &tmp_ip.s6_addr[2], &tmp_ip.s6_addr[1], &tmp_ip.s6_addr[0],
		    &tmp_ip.s6_addr[7], &tmp_ip.s6_addr[6], &tmp_ip.s6_addr[5], &tmp_ip.s6_addr[4],
		    &tmp_ip.s6_addr[11], &tmp_ip.s6_addr[10], &tmp_ip.s6_addr[9], &tmp_ip.s6_addr[8],
		    &tmp_ip.s6_addr[15], &tmp_ip.s6_addr[14], &tmp_ip.s6_addr[13], &tmp_ip.s6_addr[12]) == 16){
		    inet_ntop(AF_INET6, &tmp_ip, address, INET6_ADDRSTRLEN);
			long p = strtol(port, &ptr, 16);
			char p1[15];
	        	sprintf(p1, ":%ld", p);
	        	strcat(address, p1);
		}

    }
}
void freeList(){
    LISTELEMENT *i;
    for( i = lHead; i != NULL;){
        LISTELEMENT *t = i;
        i = i -> next;
	free(t);
    }
    PIDINFO *j;
    for( j = pHead; j != NULL;){
    	PIDINFO *t = j;    	
    	j = j -> next;
	free(t);
    }
}
