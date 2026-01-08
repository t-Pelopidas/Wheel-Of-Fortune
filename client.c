#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>

#define MAX_WORD_LENGTH 1024
#define NUM_OF_OPTIONS 3

char buffer[MAX_WORD_LENGTH];

int points = 0;

char* OPTION_ARRAY[NUM_OF_OPTIONS] = {"SPIN\n", "GUESS_LETTER\0", "GUESS_WORD\0"};

void die(const char* msg){
    perror(msg);
    exit(EXIT_FAILURE);
}

void read_line(int server_fd, char *buffer){
    int i = 0;
    char c;
    int n;

    while (1) {
        if (i >= MAX_WORD_LENGTH - 1) {
            buffer[i] = '\0';
            printf("Warning: Message too long, truncated.\n");
            return;
        }
        n = recv(server_fd, &c, 1, 0);
        if (n < 0) die("recv failed");
        if (n == 0) die("Server disconnected unexpectedly");

        buffer[i] = c;

        if (c == '\0') {
            return;
        }

        i++;
    }
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

    char message[MAX_WORD_LENGTH];

    while(1){
        read_line(server_fd, message);
        if(strncmp(message, "YOUR_TURN", strlen("YOUR_TURN")) == 0){
            printf("MY TURN\n");
            return;
        }
        printf("Host: %s\n", message);
        fflush(stdout);
    }


}

void take_a_turn(int server_fd){

    char option[MAX_WORD_LENGTH];

    while(1){

        printf("> : ");
        fgets(option, MAX_WORD_LENGTH, stdin);

        if(!strncmp(option, "SPIN",strlen("SPIN"))){

            if(send(server_fd, option, strlen(option), 0) < 0) die("Failed to send message"); 

            read_line(server_fd, buffer);
            printf("After spin: %s\n", buffer);

            if(!strncmp(buffer, "bankruptcy", strlen("bankruptcy"))){
                printf("Bankruptcy you lose your points and turn :(\n");
                points = 0;
                return;
            }

            if(!strncmp(buffer, "end_of_turn", strlen("end_of_turn"))){
                printf("You lose your turn :(\n");
                return;
            }


            int points_received = 0;
            for(int i = 0; i < (int)strlen(buffer); i++) points_received += buffer[i] - '0';
            points += points_received;

            printf("You got %d points!(Total points: %d)\n ", points_received, points);

            printf("(GUESS LETTER/WORD)> : ");
            fgets(option, MAX_WORD_LENGTH, stdin);

            while(strncmp(option, "GUESS_LETTER", strlen("GUESS_LETTER")) && strncmp(option, "GUESS_WORD", strlen("GUESS_WORD"))){
                printf("Option doesnt exits, try again\n");
                printf("(GUESS LETTER/WORD)> : ");
                fgets(option, MAX_WORD_LENGTH, stdin);
            }

            if(!strncmp(option, "GUESS_LETTER", strlen("GUESS_LETTER"))){
                char letter_to_guess = option[strlen("GUESS_LETTER") + 1];
                if(letter_to_guess == 0){
                    printf("You have to guess a letter\n");
                    return;
                }
                printf("letter to guess: %d\n", letter_to_guess);
                send(server_fd, &letter_to_guess, 1, 0);
            }
            else if(!strncmp(option, "GUESS_WORD", strlen("GUESS_WORD"))){
                char word_to_guess[MAX_WORD_LENGTH] = {0};
                int i = 0;

                while(option[strlen("GUESS_WORD") + 1 + i] != '\0'){
                    word_to_guess[i] = option[strlen("GUESS_WORD") + 1 + i];
                    i++;
                }
                word_to_guess[i] = '\0';
                send(server_fd, word_to_guess, strlen(word_to_guess), 0);
            }

            char response[MAX_WORD_LENGTH];
            read_line(server_fd, response);

            if(!strncmp(response, "no_match", strlen("no_match"))){
                printf("No match found :(\n");
                return;
            }
            else if(!strncmp(response, "word_found", strlen("word_found"))){
                printf("Word found!!\n");
                return;
            }

            else{
                printf("Match found!\n");
                printf("The Unsolved word is: \"%s\"\n", response);
            }

        }
        else{
            printf("Option doesnt exist, try again\n"); 
        }

    }

}
int main(int argc,char* argv[]){



    /*
     *  TODO:   Fix the points(lower than they must be)
     *          End the program once a player found the word correctly
     *          Random word picker
     *          Lose a turn if guessing the same letter ( maybe )
     *
     */
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

    char welcome_message[MAX_WORD_LENGTH] = {0};
    read_line(server_fd, welcome_message);
    printf("%s", welcome_message);

    printf("LET THE GAME BEGIN\n");
    
    read_line(server_fd, buffer);
    printf("The word to guess is: %s\n", buffer);

    while(strcmp(buffer,"END")){

        read_line(server_fd, buffer);
        printf("%s", buffer);

        wait_for_turn(server_fd);

        take_a_turn(server_fd);

    }

    printf("Total points: %d\n", points);

    close(server_fd);
}
