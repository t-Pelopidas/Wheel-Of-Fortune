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

char* OPTION_ARRAY[NUM_OF_OPTIONS] = {"SPIN\n", "GUESS_LETTER\0", "GUESS_WORD\0"};

void die(const char* msg){
    perror(msg);
    exit(EXIT_FAILURE);
}

char* read_line(int server_fd, char *buffer){
    int i = 0;
    char c;
    int n;

    while (1) {
        if (i >= MAX_WORD_LENGTH - 1) {
            buffer[i] = '\0'; // Force termination
            printf("Warning: Message too long, truncated.\n");
            return buffer;
        }
        n = recv(server_fd, &c, 1, 0);
        if (n < 0) die("recv failed");
        if (n == 0) die("Server disconnected unexpectedly");

        buffer[i] = c;

        if (c == '\0') {
            return buffer;
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

    char *message = read_line(server_fd,buffer);

    while(1){
        if(strcmp(message,"YOUR_TURN\0") == 0){
            printf("My turn now!\n");
            return;
        }
        printf("Waiting for turn...\r");
        printf("Host: %s", message);
        message = read_line(server_fd, buffer);
    }

    return;
}

void take_a_turn(int server_fd){

    char option[MAX_WORD_LENGTH];

    while(1){
        printf("> : ");
        fgets(option, MAX_WORD_LENGTH, stdin);

        if(strncmp(option, OPTION_ARRAY[0],strlen(OPTION_ARRAY[0])) == 0){
            if(send(server_fd, option, strlen(option), 0) < 0) die("Failed to send message");
            read_line(server_fd, buffer);
        }

        else if(strncmp(option, OPTION_ARRAY[1],strlen(OPTION_ARRAY[1])) == 0){ }

        else if(strncmp(option, OPTION_ARRAY[2],strlen(OPTION_ARRAY[2])) == 0){ }

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
    char* welcome_message = read_line(server_fd, buffer);
    printf("%s", welcome_message);

    while(strcmp(buffer,"END")){
        //WAIT TO RECEIVE TURN
        wait_for_turn(server_fd);
        
        //RECEIVE UNSOLVED WORD
        char *unsolved_word = read_line(server_fd, buffer);
        printf("The Unsolved word is: \" %s \"\n", unsolved_word);

        //SEND THE OPTION
        take_a_turn(server_fd);

        //RESPONSE
        char *response = read_line(server_fd, buffer);
        printf("%s", response);

    }

    close(server_fd);
}


//option1 = 12
//GUESS_LETTER L
//           ^
//           12
//          [11]
//
//
//
//
//

/*int handle_option(const char* option){

    int option_len = strlen(option);
    int option1 = strlen(OPTION_ARRAY[1]);
    int option2 = strlen(OPTION_ARRAY[2]);

    if(!strcmp(option,OPTION_ARRAY[0])) return 1;

    char a[option1];
    char b[option2];

    int i = 0;
    while(i <= option1 + 1){
        a[i] = option[i];
        i++;
    }
    a[option1] = '\0';

    printf("a : %s", a);

    if(strcmp(a, OPTION_ARRAY[1]) == 0 &&  option[option1] == ' ') { 
            printf("\n%d\n%d\n", option[option1], option[option1 + 1]);
            char letter_to_guess = option[option1 + 1];
            if(option[option1 + 2] != '\n'){
                printf("You can guess only one letter\n"); 
                return 0;
            }
            printf("Letter: %c, atoi(Letter): %d\n", letter_to_guess, atoi(&letter_to_guess));
            return (int)letter_to_guess;
    }
    else {
        printf("\n%d\n%d\n", option[option1], option[option1 + 1]);
        return 0;
    }


    i = 0;

    while(i <= option2){
        b[i] = option[i];
        i++;
    }
    b[option2 - 1] = '\0';


    printf("b : %s", b);

    if(!strcmp(b, OPTION_ARRAY[2]) && option[option2] == ' ') { 
        char word_to_guess[MAX_WORD_LENGTH];
        for(i = option2 + 2; i < option_len; i++){
            word_to_guess[i - option2 - 2] = option[i];  
        }
        word_to_guess[option_len - option2 + 2] = '\0';
        printf("Word to guess: %s", word_to_guess);
        return atoi(word_to_guess);

    }

    return 0;
}
*/
