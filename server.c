#include <asm-generic/socket.h>
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
#define OPENING_QUOTE "-------------\nWELCOME TO THE WHEEL OF FORTUNE\n-------------\n\0"

char r_buffer[MAX_WORD_LENGHT] = {0};
char w_buffer[MAX_WORD_LENGHT] = {0};

void die(const char* msg){
    perror(msg);
    exit(EXIT_FAILURE);
}

typedef struct {
    int client_fds[MAX_PLAYERS];
    int client_turn[MAX_PLAYERS];
    char word_to_guess[MAX_WORD_LENGHT];
    char masked_word[MAX_WORD_LENGHT];
    bool isSolved;
}GameState;

GameState init_game(){
    GameState G;
    strcpy(G.word_to_guess,"Hello\0");
    strcpy(G.masked_word,"_____\0");
    G.isSolved = false;
    return G;
}

void send_to(int player_fd,const char* msg){
    if(send(player_fd, msg, strlen(msg) + 1, 0) < 0) die("Failed to send message");
}

void broadcast(GameState G,const char* msg){

    for(int i = 0; i<MAX_PLAYERS; i++){
        send_to(G.client_fds[i], msg);
    }
    return;
}
void accept_clients(GameState *G,int *server_fd, struct sockaddr_in server_address){

    socklen_t server_address_size = sizeof(server_address);

    for(int i = 0; i<MAX_PLAYERS; i++){
        if ((G->client_fds[i] = accept(*server_fd, (struct sockaddr *)&server_address, &server_address_size)) < 0 ) die("accept failed"); 
        printf("P%d accepted with fd:%d \n", i, G->client_fds[i]);
        G->client_turn[i] = 0;
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

    int opt = 1;
    if(setsockopt(*server_fd, SOL_SOCKET, SO_REUSEADDR, &opt , sizeof(opt)) < 0) die("setsockopt failed");

    if(bind(*server_fd,(struct sockaddr *)&server_addr, server_addr_size) < 0) die("Bind failed");
    printf("Bind Completed\n");

    if(listen(*server_fd,MAX_PLAYERS) < 0) die("Listen failed");
    printf("Listen Completed\n");

    return server_addr;

}

void get_option(int server_fd,char *buf){
    int r_bytes = 0;

    if((r_bytes = recv(server_fd, buf, MAX_WORD_LENGHT - 1, 0)) < 0) die("Failed to receive message");
    buf[r_bytes]= '\0';
    printf("I got the option:\n%s\n", buf);
}

int process_option(int player_fd, const char* option){

    printf("option: %s\n", option);

    if(strcmp(option,"END\n") == 0){
        printf("option hit!\n");
        send_to(player_fd, "too bad\n\0");
        return 0;
    }

    if(strcmp(option,"SPIN\n") == 0){
        printf("option hit!\n");
        send_to(player_fd,"You got 200 points\n\0");
        return 1;
    }


    printf("option miss!\n");
    return 0;
}

int main(){

    GameState Game = init_game();

    int server_fd;

    struct sockaddr_in server_addr = init_server(&server_fd);

    accept_clients(&Game, &server_fd, server_addr);

    //WELCOME MESSAGE
    broadcast(Game, OPENING_QUOTE);

    print_game_state(Game);

    while(Game.isSolved != true){

        char response[MAX_WORD_LENGHT];

        for(int i = 0; i < MAX_PLAYERS; i++){
            Game.client_turn[i] = 1; 
            while(Game.client_turn[i]){
                //GIVE TURN
                send_to(Game.client_fds[i], "YOUR_TURN\0");

                //SEND UNSOLVED WORD
                send_to(Game.client_fds[i],Game.masked_word);
                
                //READ THE PLAYER OPTIO\\\\\nnnnnN
                get_option(Game.client_fds[i], response);

                //PROCESS THE PLAYER OPTION
                //AND SEND RESPONSE
                Game.client_turn[i] &= process_option(Game.client_fds[i], response);
            }

        }
        Game.isSolved = true;

    }
    broadcast(Game,"END\0");

    close_clients(Game);

    close(server_fd);
}
