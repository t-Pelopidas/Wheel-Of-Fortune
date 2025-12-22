#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>

#define MAX_WORD_LENGTH 1024

char buffer[MAX_WORD_LENGTH];

void die(const char* msg){
    perror(msg);
    exit(EXIT_FAILURE);
}

struct sockaddr_in init_client(int *server_fd,char* ip, char* port){

    struct sockaddr_in server_addr;

    if((*server_fd = socket(AF_INET,SOCK_STREAM, 0)) < 0 ) die("Socket Failed");
    printf("Socket Completed\n");

    server_addr.sin_family = AF_INET;

    int int_port = atoi(port);
    server_addr.sin_port = htons(int_port);

    if(inet_aton(ip, &server_addr.sin_addr) == 0) die("Conversion from ARGUMENT to ADDRESS failed");
    printf("inet_aton Completed\n");
    
    return server_addr;
} 

void wait_for_turn(int server_fd){
    int r_bytes;
    char b[MAX_WORD_LENGTH];

    if((r_bytes = recv(server_fd, b, strlen("YOUR_TURN"), 0)) <= 0) die("Receive failed");;
    while(strcmp(b,"YOUR_TURN") != 0){
        printf("Waiting\n");
        if((r_bytes = recv(server_fd, b, MAX_WORD_LENGTH, 0)) <= 0) die("Receive failed");;
    }

    printf("My turn now!\n");
    return;
}

void take_a_turn(int server_fd){

    char option[MAX_WORD_LENGTH];

    while(1){
        printf("My turn: ");
        fgets(option, MAX_WORD_LENGTH, stdin);
        if(!strcmp(option,"SPIN\n")){
            send(server_fd, option, strlen(option), 0);
            return;
        }
        printf("Option doesnt exist, try again\n");
    }


}
int main(int argc,char** argv){

    if(argc < 3) {
        printf("Usage: %s <IP Address> <Port>\n", argv[0]);
        die("Wrong Usage");
    }
    int server_fd, r_bytes;

    struct sockaddr_in server_addr = init_client(&server_fd, argv[1], argv[2]);
    socklen_t server_addr_len = sizeof(server_addr);


    if (connect(server_fd,(struct sockaddr *)&server_addr,server_addr_len) < 0) die("Connect Failed");
    printf("Connection Established\n");

    printf("%s, %d, %d\n", inet_ntoa(server_addr.sin_addr), htons(server_addr.sin_port), server_fd);

    if((r_bytes = recv(server_fd, buffer, MAX_WORD_LENGTH, 0)) < 0) die("Failed to receive message");
    printf("%s", buffer);

    while(strcmp(buffer,"END")){

        wait_for_turn(server_fd);

        take_a_turn(server_fd);

        if((r_bytes = recv(server_fd, buffer, MAX_WORD_LENGTH, 0)) < 0) die("Failed to receive message");

    }
    printf("End of game\n");

    close(server_fd);

}
