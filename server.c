#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdbool.h>

#define PORT 4001
#define MAX_PLAYERS 1
#define MAX_WORD_LENGHT 1024
#define OPENING_QUOTE "HELLO EVERONE\nWELCOME TO THE WHEEL OF FORTUNE\nITS TIME TO START THE GAME\n"

char buffer[MAX_WORD_LENGHT];

void die(const char* msg){
    perror(msg);
    exit(EXIT_FAILURE);
}

typedef struct {
    int client_fds[MAX_PLAYERS];
    char word_to_guess[MAX_WORD_LENGHT];
    char masked_word[MAX_WORD_LENGHT];
    bool isSolved;
}GameState;

GameState init_game(){
    GameState G;
    strcpy(G.word_to_guess,"Hello");
    strcpy(G.masked_word,"_____");
    G.isSolved = false;
    return G;
}

void send_to(int fd, const char* msg){
    if(send(fd, msg, strlen(msg), 0) < 0) die("Failed to send message");
}

void broadcast(GameState G,const char* msg){

    for(int i = 0; i<MAX_PLAYERS; i++){
        send_to(G.client_fds[i], msg);
    }
    return;
}
void accept_clients(GameState *G,int server_fd, struct sockaddr_in server_address){

    socklen_t server_address_size = sizeof(server_address);

    for(int i = 0; i<MAX_PLAYERS; i++){
        if ((G->client_fds[i] = accept(server_fd, (struct sockaddr *)&server_address, &server_address_size)) < 0 ) die("accept failed"); 
        printf("P%d accepted with fd:%d \n", i, G->client_fds[i]);
    }

}

void print_game_state(GameState G){

    printf("-----GAME STATE-----\n");
    printf("client_fds: ");
    for (int i = 0; i<MAX_PLAYERS; i++) {
        printf("%d ",G.client_fds[i]);
    }
    printf("\n");
    printf("isSolved: %d\n", G.isSolved);
    printf("Word: %s\n", G.word_to_guess);
    printf("Masked Word: %s\n", G.masked_word);
    printf("--------------------\n");

}
void close_clients(GameState G){
    
    for(int i = 0; i<MAX_PLAYERS; i++){
        close(G.client_fds[i]);
    }

    return;
}
struct sockaddr_in init_server(int *server_fd){
    struct sockaddr_in server_addr;

    if((*server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) die("Server socket failed");
    printf("Socket Completed\n");
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    socklen_t server_addr_size = sizeof(server_addr);

    printf("%d, %d, %d, %d\n", server_addr.sin_family, server_addr.sin_addr.s_addr, server_addr.sin_port, *server_fd);

    if(bind(*server_fd,(struct sockaddr *)&server_addr, server_addr_size) < 0) die("Bind failed");
    printf("Bind Completed\n");

    if(listen(*server_fd,MAX_PLAYERS) < 0) die("Listen failed");
    printf("Listen Completed\n");

    return server_addr;

}
int main(){

    GameState Game = init_game();

    int server_fd, r_bytes;

    struct sockaddr_in server_addr = init_server(&server_fd);

    accept_clients(&Game,server_fd, server_addr);

    broadcast(Game,OPENING_QUOTE);

    print_game_state(Game);

    if ((r_bytes = recv(Game.client_fds[0], buffer,MAX_WORD_LENGHT, 0)) < 0) die("Failed to receive message");
    buffer[r_bytes] = '\0';

    while(strcmp(buffer, "QQ") != 0){

        printf("From P%d: %s",0, buffer);

        fflush(stdin);
        fgets(buffer, MAX_WORD_LENGHT, stdin);

        send_to(Game.client_fds[0],buffer);

        if ((r_bytes = recv(Game.client_fds[0], buffer,MAX_WORD_LENGHT, 0)) < 0) die("Failed to receive message");
        buffer[r_bytes] = '\0';
    }
    close_clients(Game);

    close(server_fd);

}
