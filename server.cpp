#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/types.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <map> 
#include <bits/stdc++.h>

using namespace std;

#define SERVERPORT 8989
#define BUFSIZE 4096
#define SOCKETERROR (-1)
#define SERVERBACKLOG 1000

#define THREAD_POOL_SIZE 20

typedef struct sockaddr_in SA_IN;
typedef struct sockaddr SA;

pthread_t thread_pool[THREAD_POOL_SIZE];
 int check(int exp,const char *msg){
	if(exp == SOCKETERROR){
		perror(msg);
		exit(1);
	}
}
// void handleConnection(int client_socket){
// 	char buffer[BUFSIZE];
// 	size_t bytes_read;
// 	int msgsize=0;
// 	char actualpath[PATH_MAX+1];

// 	//read the clients message -- name of the file to read

// 	while((bytes_read = read(client_socket,buffer+msgsize,sizeof(buffer)-msgsize-1)) > 0){
// 		msgsize+=bytes_read;

// 		if(msgsize > BUFSIZE-1 || buffer[msgsize-1] == '\n') break;

// 	}
// 	check(bytes_read,"recv error");
// 	buffer[msgsize-1] = 0 ;//null terminate the msg and remove the \n
// 	cout<<"REQUEST: "<<buffer<<endl;
// 	fflush(stdout);

// 	//validity check

// 	if(realpath(buffer,actualpath) == NULL){
// 		cout<<" error badpath "<<buffer<<endl;
// 		close(client_socket);
// 		return;
// 	}

// 	//read file and send its contents to the client

// 	FILE *fp=fopen(actualpath,"r");
// 	if(fp == NULL){
// 		cout<<" error open "<<buffer<<endl;
// 		close(client_socket);
// 		return;
// 	}

// 	//read file contents and send them to client 
// 	while((bytes_read = fread(buffer,1,BUFSIZE,fp))>0){
// 		cout<<" sending bytes "<< bytes_read<<endl;
// 		write(client_socket,buffer,bytes_read);

// 	}
// 	close(client_socket);
// 	fclose(fp);
// 	cout<<" closing connection "<<endl;


// }


void *handleConnection(void *p_client_socket){
	cout<<" here "<<endl;
	 int client_socket=*((int*)p_client_socket);
	free( p_client_socket);
	   
	  
        char buffer[1024] = {0}; 

        int valread = read( client_socket , buffer, 1024); 
        printf(" message from client : ");
	    printf("%s\n",buffer ); 
	    string s;
	   cin>>s;
	   char *hello=new char[s.length() + 1]; 
	   strcpy(hello, s.c_str());
	    send(client_socket , hello , strlen(hello) , 0 ); 
	    printf(" message sent\n");
	    cout<<"\n \n"; 
	    return NULL;
}

int main(int argc,char **argv){

	int server_socket,client_socket,addr_size;
	SA_IN serv_addr,client_addr;

   //creating a bunch of threads for future connections
// 	for (int i=0;i<THREAD_POOL_SIZE;i++){
// 		pthread_create(&thread_pool[i],NULL,threadFunction,NULL);
// }
	server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (server_socket < 0){

        cout << "SOCKET ERROR\n";
	}


	//initialise the adress struct

	serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERVERPORT);
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
   
    //check bind socket

    int b = bind(server_socket, (SA*)&serv_addr, sizeof(serv_addr));
    if (b < 0)
    {
        //cout << userid << ":" << userpass << "\n";
        cout << "Binding Error\n";
        exit(1);
    }
    cout << "Binding created\n";

     if (listen(server_socket, SERVERBACKLOG) < 0)
            cout << "LISTENING ERROR\n";
        cout << "Listening created\n";


     while(true){

     	cout<<"Waiting for connections "<<endl;
     	addr_size=sizeof(SA_IN);
     	 client_socket = accept(server_socket, (SA*)&client_addr, (socklen_t*)&addr_size);
        if (client_socket < 0)
        {
            cout << "CONNECTION FAILED\n";
            exit(1);
        }
        else
            cout << "CONNECTION SUCCESSFUL\n";

         // handleConnection(client_socket);
 		int *pclient=new int;
 		*pclient=client_socket;
         pthread_t t;
         // pthread_create(&t,NULL,threadFunction,argument)
         pthread_create(&t,NULL,handleConnection,pclient);
        // sleep(2000);
     }
}