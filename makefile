all: hangman_server hangman_client

hangman_server: hangman_server.c
	gcc -o hangman_server hangman_server.c

hangman_client: hangman_client.c
	gcc -o hangman_client hangman_client.c

clean:
	rm -f *.o server *~
	rm -f *.o hangman_client *~