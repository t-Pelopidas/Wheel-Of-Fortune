#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdbool.h>

void die(const char* msg){
    perror(msg);
    exit(EXIT_FAILURE);
}


int main(int argc, char** argv){

    if(argc < 2) {
        printf("Usage: ./<program> <IP Address> <Port>\n");
        die("Wrong Usage");
    }

    printf("IP: %s, Port: %d\n",argv[1], atoi(argv[2]));
}
