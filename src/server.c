#include <asm-generic/socket.h>
#include <stddef.h>
#include <strings.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

#define PORT 4001
#define MAX_PLAYERS 3
#define MAX_WORD_LENGHT 1024
#define OPENING_QUOTE "------------------------------ WELCOME TO THE WHEEL OF FORTUNE ------------------------------\n\0"

char *WORD_ARRAY[128] ={"Actor","Amazon","Animal","Answer","Apple","Area","Artist","Asteroid","Atlantic","Audience","August","Australia","Bicycle","Biology","Birthday","Bone","Bread","Business","Camera","Captain","Century","Chef","Cinema","Classic","Coffee","Comedy","Comet","Computer","Concert","Country","Create","Dance","Decade","Desert","Dessert","Dinner","Discover","Doctor","Dolphin","Drama","Eagle","Earth","Energy","Famous","Farmer","Fashion","February","Flower","Forest","Friday","Galaxy","Garden","Giraffe","Golden","Gorilla","Gravity","History","Holiday","Honey","Imagine","Island","January","Keyboard","Kitchen","Language","Lawyer","Lemon","Leopard","Listen","Market","Memory","Midnight","Mirror","Modern","Monday","Morning","Mountain","Movie","Music","Nature","Novel","Ocean","Opera","Oxygen","Painting","Paper","Pasta","Penguin","Piano","Pilot","Pizza","Planet","Player","Poetry","Popular","President","Prize","Question","Rabbit","Radio","Remember","River","Rocket","Royal","Science","Silver","Solar","Space","Summer","Teacher","Telephone","Telescope","Television","Theater","Tiger","Tonight","Travel","Umbrella","Valley","Vintage","Volcano","Water","Waterfall","Weather","Website","Winter","Yesterday","Zebra"};

char r_buffer[MAX_WORD_LENGHT] = {0};
char w_buffer[MAX_WORD_LENGHT + 1024] = {0};


void die(const char* msg){
    perror(msg);
    exit(EXIT_FAILURE);
}

typedef struct {
    int     client_fds[MAX_PLAYERS];
    char    word_to_guess[MAX_WORD_LENGHT];
    char    masked_word[MAX_WORD_LENGHT];
    bool    isSolved;
}GameState;

GameState init_game(){
    srand(time(NULL));

    GameState G;
    int i = rand()%128;
    snprintf(G.word_to_guess, strlen(WORD_ARRAY[i]) + 1,"%s", WORD_ARRAY[i]);
    for(int j = 0; j < (int)strlen(WORD_ARRAY[i]); j++){
        G.masked_word[j] = '_';
    }
    G.masked_word[strlen(WORD_ARRAY[i])] = '\0';

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
}

int process_option(GameState *G, int player_fd, const char* option){

    if(strncmp(option, "SPIN", strlen("SPIN")) == 0){

        int points = rand()%1501, r_bytes = 0;

        char spoints[5] = {0};

        for(int i = 0; i < 4; i++) {
            int power = 1;
            for(int j=0; j<i; j++) power *= 10; 
            int digit = (points / power) % 10;
            spoints[3-i] = digit + '0'; 
        }
        
        if(points >= 120 && points <= 220){
            send_to(player_fd, "bankruptcy");
            return 0;
        }
        if((points >= 220 && points <= 430) || (points >= 920 && points <= 1020)){
            send_to(player_fd, "end_of_turn");
            return 0; 
        }

        send_to(player_fd, spoints);

        if((r_bytes = recv(player_fd, r_buffer, MAX_WORD_LENGHT, 0))< 0) die("Failed to recv");

        if(*r_buffer == 0){
            return 0;
        }
        r_buffer[r_bytes] = '\0';

        if(strlen(r_buffer) ==  1){
            bool match_found = false;

            for(int i = 0; i < (int)strlen(G->word_to_guess); i++){
                if(G->word_to_guess[i] == *r_buffer){
                    if(G->masked_word[i] == *r_buffer){
                        send_to(player_fd, "lose_turn\0");
                        return 0;
                    }
                    G->masked_word[i] = *r_buffer;
                    match_found = true;
                }
            }
            if(!match_found){
                send_to(player_fd, "no_match\0");
                return 0;
            }
            if(!strncmp(G->word_to_guess, G->masked_word, strlen(G->word_to_guess))){
                send_to(player_fd, "word_found\0");
                G->isSolved = true;
                return 0;
            }
            else{
                broadcast(*G,G->masked_word);
                return 1;
            }
        }
        else{
            if(!strncmp(r_buffer, G->word_to_guess, strlen(G->word_to_guess))){
                send_to(player_fd, "word_found\0");
                G->isSolved = true;
                return 0;
            }
            send_to(player_fd, "no_match\0");
            return 0;
        }

    }
    else{
        return 0;
    }

    return 0;
}

int main(){

    GameState Game = init_game();

    int server_fd;

    struct sockaddr_in server_addr = init_server(&server_fd);

    accept_clients(&Game, &server_fd, server_addr);

    broadcast(Game, OPENING_QUOTE);

    broadcast(Game, Game.masked_word);

    print_game_state(Game);

    while(!Game.isSolved){

        char response[MAX_WORD_LENGHT];

        for(int i = 0; i < MAX_PLAYERS; i++){
            char current_info[32];
            sprintf(current_info,"Its Player %d's turn\n" ,i + 1);
            broadcast(Game, current_info);

            int player_turn = 1; 
            send_to(Game.client_fds[i], "YOUR_TURN\0");
            printf("Its player's %d turn\n", i);
            while(player_turn){
                get_option(Game.client_fds[i], response);
                player_turn &= process_option(&Game, Game.client_fds[i], response);
                print_game_state(Game);

            }
            if(Game.isSolved) { 
                printf("Player %d wins the game!!!\n", i + 1);
                sprintf(w_buffer, "Player %d wins the game!!! The word is %s\n", i + 1, Game.word_to_guess);
                break;
            }
        }
    }
    broadcast(Game,"END\0");
    broadcast(Game, w_buffer);

    close_clients(Game);

    close(server_fd);
}
