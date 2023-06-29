#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <ctype.h>
#include <stdbool.h>

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    

    char buffer[1024];
    char currentWord[1024];
    char badGuesses[1024];
    char start[1024];
    bool gameOver = false;
    int numBad = 0;
    printf(">>>Ready to start game? (y/n): ");
    if(fgets(start, 1024, stdin) == NULL){
        printf("\n");
        return 0;
    };
    printf(">>>");
    start[strlen(start) - 1] = '\0'; 
    if(start[0]=='n'){
        close(sockfd);
        return 0;
    }
    if(start[0]=='y'){//WHAT TO DO IF No????
        if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
            error("ERROR connecting");
        n = write(sockfd,start,1024);
        n = read(sockfd, currentWord, 1024);
        //printf(">>>%s\n",currentWord);
        for(int i = 0; i<strlen(currentWord); i++){
                    printf("%c", currentWord[i]);
                    if(i!=strlen(currentWord)-1)
                        printf(" ");
                }
                printf("\n");
        printf(">>>Incorrect Guesses: %s\n", badGuesses);
        printf(">>>\n");
    }
    while(!gameOver){
        //start with receiving, only respond if appropriate type
        // n = read(sockfd, buffer, 1024);
        // printf(">>>%s\n", currentWord);
        // printf(">>>Incorrect Guesses: %s\n", badGuesses);
        // printf(">>>\n");
        printf(">>>Letter to guess: ");
        char letter[1024];
        if(fgets(letter, 1024, stdin) == NULL){
                    printf("\n");
                    return 0;
        }
        //printf("\n");
        if(strlen(letter)>2)
            printf(">>>Error! Please guess one letter.\n");
        else if(!isalpha(letter[0]))
            printf(">>>Error! Please guess one letter.\n");
        else{
            letter[0] = tolower(letter[0]);
            //SEND LETTER TO SERVER
            n = write(sockfd,letter,1024);
            if (n < 0) 
                error("ERROR writing to socket");
            n = read(sockfd, buffer, 1024);
            //strcpy(currentWord, buffer);
            if (n < 0) 
                error("ERROR reading from socket");

            //if(strcmp(buffer,"8\n")==0){
            if(buffer[0]=='8'){
                printf(">>>The word was ");
                for(int i = 3; i<strlen(buffer); i++){
                    printf("%c", buffer[i]);
                }
                printf("\n");
                printf(">>>You Win!\n>>>Game Over!\n");
                gameOver = true;
                close(sockfd);
            }
            else if(buffer[0]=='9'){
                printf(">>>The word was ");
                for(int i = 3; i<strlen(buffer); i++){
                    printf("%c", buffer[i]);
                }
                printf("\n>>>You Lose!\n>>>Game Over!\n");
                gameOver = true;
                close(sockfd);
            }
            // else if(buffer[0]=='s'){
            //     printf("server-overloaded\n");
            // }
            else{
                //update bad guesses
                int newNumBad = buffer[2] - '0';
                //printf("new num bad: %d\n", newNumBad);
                //printf("old num bad: %d\n", numBad);
                if(numBad != newNumBad){
                    //printf("Badguesses in loop %s\n", badGuesses);
                    badGuesses[numBad] = letter[0];
                    numBad++;
                    badGuesses[numBad] = '\0';
                }
                printf(">>>");
                for(int i = 3; i<strlen(buffer); i++){
                    printf("%c", buffer[i]);
                    if(i!=strlen(buffer)-1)
                        printf(" ");
                }
                printf("\n");
                //printf(">>>???%s\n",buffer);
                // if(strcmp(buffer, currentWord)==0){//didnt change, bad guess
                // }
                printf(">>>Incorrect Guesses: ");
                for(int i = 0; i<strlen(badGuesses); i++){
                    printf("%c", badGuesses[i]);
                    if(i!=strlen(badGuesses)-1)
                        printf(" ");
                }
                printf("\n");
                printf(">>>\n");
                bzero(buffer, 1024);
                bzero(letter,1024);
            }

        }
    }
    // while(n>=1){
    //     n = read(sockfd,buffer,255);
    //     if (n < 0) 
    //         error("ERROR reading from socket");
    //     printf("n = %d\n",n);
    //     //printf("From server: %s\n",buffer);
    //     write(1,"From server: ",13);
    //     write(1,buffer,n);
    //     write(1, "\n",1);
    //     //sleep(1);
    //     if(isalpha(buffer[0])){
    //         n=-1;
    //         close(sockfd);
    //         break;
    //     }

    //     if(n==1) {
    //         close(sockfd);
    //         break;
    //     }
    // }
    //close(sockfd);
    return 0;
}