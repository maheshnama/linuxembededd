/*
C socket server example, handles multiple clients using threads
Compile
gcc server.c -lpthread -o server
*/
#include<stdio.h>
#include<string.h> //strlen
#include<stdlib.h> //strlen
#include<sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include<arpa/inet.h> //inet_addr
#include<unistd.h> //write
#include<pthread.h> //for threading , link with lpthread
#include <stdbool.h>
#define PORT_NUMBER 1888
//the thread function
void *connection_handler(void *);
pthread_mutex_t lock;
char buffer [255];

void storeData(char *receivedData)
{
	char filename[100];
	time_t t;
	FILE* fp;
	srand(time(NULL));
	sprintf(filename,"/root/TaxiMeter/taximeterdatadump/%d.txt",rand());
	fp = fopen(filename,"w");
	fputs(receivedData,fp);
	fclose(fp);  		

        
}




int main(int argc , char *argv[])
{
	int socket_desc , client_sock , c, *new_sock;
	struct sockaddr_in server , *servinfo,client;
	int ttl = 60; /* max = 255 */
	int error;
	//Create socket
	socket_desc = socket(AF_INET , SOCK_STREAM , 0);
	if (socket_desc == -1)
	{
		printf("Could not create socket");
		freeaddrinfo(servinfo);
	}
	puts("Socket created");

	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(PORT_NUMBER);
	//Bind
	if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
	{
		//print the error message
		perror("bind failed. Error");
		return 1;
	}

	puts("bind done");
        int tr=1;

	// kill "Address already in use" error message
	if (setsockopt(socket_desc,SOL_SOCKET,SO_REUSEADDR,&tr,sizeof(int)) == -1) {
    	perror("setsockopt");
    	return 1;
	}
	//Listen
	listen(socket_desc ,10);

	//Accept and incoming connection
	puts("Waiting for incoming connections...");
	c = sizeof(struct sockaddr_in);
	pthread_t thread_id;
	while( (client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
	{
		puts("Connection accepted");
	new_sock = malloc(1);
        *new_sock = client_sock;
	
	if( pthread_create( &thread_id , NULL , connection_handler , (void*) new_sock) < 0)
		{
		perror("could not create thread");
		return 1;
			}
		//Now join the thread , so that we dont terminate before the thread
		//pthread_detach(thread_id);
		puts("Handler assigned");
	}

	if (client_sock < 0)
	{
		perror("accept failed");
		close(client_sock);
		return 1;
	}
        	

	return 0;
}


/*
* This will handle connection for each client
* */
void *connection_handler(void *socket_desc)
{
	//Get the socket descriptor
	int sock = *(int*)socket_desc;
	int read_size;
	char *message , client_message[2000];
	bool result=true;
        char *ack;	
	
	//Send some messages to the client
	message = "SOC_ESTB\n";
	write(sock , message , strlen(message));
	char buffer[20];

	//Receive a message from client
	while( (read_size = recv(sock , client_message , 2000 , 0)) > 0 )
	{
		//end of string marker
		client_message[read_size] = '\0';
		printf("*** %s\n",client_message);
		storeData(client_message);
	      	message = "SRV_ACK\n";
		write(sock , message , strlen(message));
		//clear the message buffer
		memset(client_message, 0, 2000);
//		close(sock);
  //      	free(socket_desc);
        	//shutdown(sock,SHUT_RDWR);
            
	}

	if(read_size == 0)
	{
		puts("Client disconnected");
		fflush(stdout);
	}
	else if(read_size == -1)
	{
		perror("recv failed");
	}
	close(sock);
	free(socket_desc);

	shutdown(sock,SHUT_RDWR);

	return 0;
} 
