/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include <sys/mman.h>
//global vars
int* activeGames;
pthread_mutex_t* activeGamesMutex;

struct gamedeets {
  char answer[10];
  char currentword[10];
  char incorrect[9];
  int numIncorrect;
  int numCorrect;
};

void error(const char *msg)
{
    perror(msg);
    exit(1);
}
void generateAnswer(struct gamedeets *game, int randInt){
    //srand(time(NULL));
    //printf("ingen\n");
    int line_num = randInt;
    FILE* file;
    file = fopen("hangman_words.txt", "r");
    //printf("ingen\n");
    for(int i = 0; i < line_num; i++){
        fgets(game->answer, 100, file);
    }
    game->answer[strlen(game->answer)-1] = '\0';
    for(int i = 0; i < strlen(game->answer)-1; i++){
        game->answer[i] = tolower(game->answer[i]);
    }
    //printf("ingen\n");
    fclose(file);
}
void initCurrentWord(struct gamedeets *game, int len){
    for(int i =0; i<len; i++){
        game->currentword[i]='_';
    }
    game->currentword[len] = '\0';
}
void checkLetter(char* letter, struct gamedeets *game){
    bool inWord = false;
    if((strchr(game->currentword, letter[0]))){
        game->numIncorrect++;
    }
    else{
        for(int i = 0; i< strlen(game->answer); i++){
            if(game->answer[i] == letter[0]){
                    game->currentword[i] = letter[0];
                    game->numCorrect++;
                    inWord = true;   
            }
        }
        if(!inWord){
                //game->incorrect[game->numIncorrect] = letter[0];
                game->numIncorrect++;
            }
    }
}

void runGame(int sock, int randInt){
    //printf("ingame\n");
    struct gamedeets game1;
    //printf("ingame2\n");
    game1.numIncorrect = 0;
    game1.numCorrect = 0;
    //printf("ingame3\n");
    generateAnswer(&game1, randInt);
    //printf("ingame4\n");
    initCurrentWord(&game1, strlen(game1.answer));
    //printf("ingame5\n");
    printf("%s\n", game1.answer);
    
    if (sock < 0)
            error("ERROR on accept");
    //bzero(buffer,1024);
    int n = 0;
    bool started = false;
    char* buffer = malloc(1024);
    while(!started){
        n = recv(sock,buffer,1024,0);
        if(buffer[0]=='y'){
            started = true;
            bzero(buffer,1024);
            //sprintf(buffer, "%c%c%c%s", '0', strlen(game1.answer) + '0', game1.numIncorrect + '0', game1.currentword);
            n = write(sock, game1.currentword, 1024);
        }
    }
    while(game1.numIncorrect < 6 && game1.numCorrect < strlen(game1.answer)){
        n = recv(sock,buffer,1024,0);
        if (n < 0) 
            error("ERROR reading from socket");
        printf("%s\n",game1.currentword);
        buffer[0] = tolower(buffer[0]);
        printf("current letter received: %s\n", buffer);
        checkLetter(buffer, &game1);
        //bzero(buffer,1024);
        //let client handle corresponding prints
        if(game1.numCorrect == strlen(game1.answer)){//just send flag for winning
            //send 2 packets? send the word
            //sprintf(buffer, "8\n");
            sprintf(buffer, "%c%lu%c%s", '8', strlen(game1.answer), game1.numIncorrect + '0', game1.currentword);//might need + '0' for length

            n = write(sock, buffer, 1024);
            bzero(buffer,1024);
        }
        else if(game1.numIncorrect == 6){//just send flag for losing
            //sprintf(buffer, "9\n");
            sprintf(buffer, "%c%lu%c%s", '9', strlen(game1.answer), game1.numIncorrect + '0', game1.answer);//might need + '0' for length
            n = write(sock, buffer, 1024);
            bzero(buffer,1024);
        }
        else{//send word length, num INcorrect, and updated string
            sprintf(buffer, "%c%lu%c%s", '0', strlen(game1.answer), game1.numIncorrect + '0', game1.currentword);
            printf("%s\n",buffer);
            n = write(sock, buffer, 1024);
            bzero(buffer,1024);
        }
        
        //printf("numcorrect %d :\n", game1.numCorrect);
        //printf("numincorrect %d :\n\n", game1.numIncorrect);
    }
    sleep(1);
    free(buffer);
    
}
int main(int argc, char *argv[])
{   
    int maxClients = 6;
    int clientSock[maxClients];
    int sockfd, newsockfd, portno; //newsockfd was client sock before
    socklen_t clilen;
    char buffer[1024];
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }
    for(int i = 0; i < maxClients; i++){
        clientSock[maxClients] = 0;
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0); //primary socket
    int opt = 1;
    n = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt));////
    
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
        error("ERROR on binding");
    
    if (listen(sockfd, SOMAXCONN)) //5?
    {  
        perror("listen error");
        return 1;
    }
    clilen = sizeof(cli_addr);
    activeGames = mmap(NULL, sizeof *activeGames, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0); 
    activeGamesMutex = mmap(NULL, sizeof *activeGamesMutex, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0); 
    *activeGames = 0;
    pthread_mutex_t temp = PTHREAD_MUTEX_INITIALIZER;
    *activeGamesMutex = temp;

    //run server coninuously below here
    while(true){
        //printf("made it to the loop\n");
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        int randInt = rand() % 15;
        if (sockfd < 0) 
            error("ERROR opening socket");
        //printf("after the accept\n");
        pthread_mutex_lock(activeGamesMutex);
        int numActive = *activeGames;
        pthread_mutex_unlock(activeGamesMutex);
        if(numActive >=3){
            write(newsockfd, "server-overloaded\n", 30);
            close(newsockfd);
            continue;
        }
        else{
            //printf("in the else\n");
            //write(newsockfd, " ", 69);
            pthread_mutex_lock(activeGamesMutex);
            *activeGames+=1;
            pthread_mutex_unlock(activeGamesMutex);
        }
        //printf("right before fork\n");
        pid_t pid = fork();

        if(pid == -1){
            return 1;
        }
        else if(pid == 0){
            close(sockfd);
            runGame(newsockfd, randInt);
            pthread_mutex_lock(activeGamesMutex);
            *activeGames-=1;
            pthread_mutex_unlock(activeGamesMutex);
            close(newsockfd);
            return 0;   
        }
    }
    return 0;
    
}