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
char* get_message(int server_fd, char *buffer){
    int r_bytes;
    if((r_bytes = recv(server_fd, buffer, MAX_WORD_LENGTH, 0)) < 0) die("Failed to receive message");
    buffer[r_bytes] = '\0';
    return buffer;
}

struct sockaddr_in init_client(int *server_fd, char* ip, char* port){

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

    printf("Waiting for turn...\n");
    char *message = get_message(server_fd,buffer);
    printf("wait: %s\n", buffer);
    while(strcmp(message,"YOUR_TURN\0") != 0){
        printf("Waiting for turn...\r");
        strcpy(message,get_message(server_fd, buffer));
    }
    printf("My turn now!\n");

    return;
}
void take_a_turn(int server_fd){

    char option[MAX_WORD_LENGTH];

    while(1){
        printf("> :  ");
        fgets(option, MAX_WORD_LENGTH, stdin);
        if(!strcmp(option,"SPIN\n") || !strcmp(option,"END\n")){
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
    int server_fd;

    struct sockaddr_in server_addr = init_client(&server_fd, argv[1], argv[2]);
    socklen_t server_addr_len = sizeof(server_addr);


    if (connect(server_fd,(struct sockaddr *)&server_addr,server_addr_len) < 0) die("Connect Failed");
    printf("Connection Established\n");

    printf("%s, %d, %d\n", inet_ntoa(server_addr.sin_addr), htons(server_addr.sin_port), server_fd);

    //WELCOME MESSAGE
    char* welcome_message = get_message(server_fd,buffer);
    printf("%s", welcome_message);

    while(strcmp(buffer,"END")){
        //WAIT TO RECEIVE TURN
        printf("\nbefore wait\n");
        wait_for_turn(server_fd);
        printf("\nafter wait\n");
        
        //RECEIVE UNSOLVED WORD
        char *unsolved_word = get_message(server_fd,buffer);
        printf("The Unsolved word is: \" %s \"\n", unsolved_word);

        //SEND THE OPTION
        printf("\nbefore take_a_turn\n");
        take_a_turn(server_fd);
        printf("after take_a_turn\n");

        //RESPONSE
        printf("\nbefore response\n");
        char *response = get_message(server_fd,buffer);
        printf("after response\n");
        printf("%s", response);

    }

    close(server_fd);
}
