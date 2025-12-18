#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdbool.h>

#define PORT 4001
#define MAX_PLAYERS 3
#define MAX_WORD_LENGHT 16

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
    write(fd, msg, strlen(msg));

    return;
}

void broadcast(GameState G,const char* msg){

    for(int i = 0; i<MAX_PLAYERS; i++){
        send_to(G.client_fds[i],msg);
    }

    return;
}
void accept_clients(GameState *G,int server_socket, struct sockaddr_in server_address){


    socklen_t server_address_size = sizeof(server_address);

    for(int i = 0; i<MAX_PLAYERS; i++){
        if ((G->client_fds[i] = accept(server_socket, (struct sockaddr *)&server_address, &server_address_size)) < 0 ) die("accept failed"); 
        printf("P%d accepted with fd:%d \n", i, G->client_fds[i]);
    }

}

void print_game_state(GameState G){

    printf("-----GAME STATE-----\n\n");
    printf("client_fds: ");
    for (int i = 0; i<MAX_PLAYERS; i++) {
        printf("%d ",G.client_fds[i]);
    }
    printf("\n");
    printf("isSolved: %d\n", G.isSolved);
    printf("Word: %s\n", G.word_to_guess);
    printf("Masked Word: %s\n", G.masked_word);
    printf("\n");
    printf("--------------------\n");

}
void close_clients(GameState G){
    
    for(int i = 0; i<MAX_PLAYERS; i++){
        close(G.client_fds[i]);
    }

    return;
}

void init_server(struct sockaddr_in* server_addr, int *server_socket){
    if((*server_socket = socket(AF_INET,SOCK_STREAM,0)) < 0) die("Server socket failed");

    server_addr->sin_family = AF_INET;
    server_addr->sin_port = htons(PORT);
    server_addr->sin_addr.s_addr = INADDR_ANY;
    socklen_t server_addr_size = sizeof(*server_addr);

    //Bind failed: address family not supported by protocol
    if(bind(*server_socket,(struct sockaddr *)&server_addr, server_addr_size) < 0) die("Bind failed");
    printf("Bind Completed\n");

    if(listen(*server_socket,MAX_PLAYERS) < 0) die("Listen failed");
    printf("Listen Completed\n");

}
int main(){
    GameState Game = init_game();
    struct sockaddr_in server_addr;
    int server_socket;

    print_game_state(Game);

    init_server(&server_addr, &server_socket);

    printf("Socket Completed\n");

    accept_clients(&Game,server_socket, server_addr);

    broadcast(Game,"HELLO EVERONE\n");

    print_game_state(Game);

    close_clients(Game);

    close(server_socket);

}
