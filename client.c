#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>

#define MAX_WORD_LENGTH 1024

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

int main(int argc,char** argv){

    char buffer[MAX_WORD_LENGTH];
    if(argc < 3) {
        printf("Usage: ./<program> <IP Address> <Port>\n");
        die("Wrong Usage");
    }
    int server_fd;

    struct sockaddr_in server_addr = init_client(&server_fd, argv[1], argv[2]);
    socklen_t server_addr_len = sizeof(server_addr);


    if (connect(server_fd,(struct sockaddr *)&server_addr,server_addr_len) < 0) die("Connect Failed");
    printf("Connection Established\n");

    printf("%s, %d, %d\n", inet_ntoa(server_addr.sin_addr), htons(server_addr.sin_port), server_fd);

    int r_bytes;

    if((r_bytes = recv(server_fd,buffer, MAX_WORD_LENGTH, 0)) < 0) die("Failed to receive message");
    buffer[r_bytes] = '\0';

    while(1){

        printf("From server: %s", buffer);

        fflush(stdout);
        printf("To Server: ");
        fflush(stdin);
        fgets(buffer, MAX_WORD_LENGTH, stdin);

        if(send(server_fd, buffer, strlen(buffer), 0) < 0 ) die("Failed to send message");

        if((r_bytes = recv(server_fd,buffer, MAX_WORD_LENGTH, 0)) < 0) die("Failed to receive message");
        buffer[r_bytes] = '\0';
    }

}
