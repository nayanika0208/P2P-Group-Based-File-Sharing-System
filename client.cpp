
// Client side C/C++ program to demonstrate Socket programming 
#include <stdio.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string.h> 
#include<bits/stdc++.h>
#define PORT 8989
   
 using namespace std;  
int main(int argc, char const *argv[]) 
{ 
    for (int i=0;i<100;i++){
        int sock = 0, valread; 
        struct sockaddr_in serv_addr; 
        // string h="Hello from client "+ to_string(i);
          string h;
        cin>>h;
        char *hello = new char[h.length() + 1];
        strcpy(hello, h.c_str());
       
        char buffer[1024] = {0}; 
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
        { 
            printf("\n Socket creation error \n"); 
            return -1;  
        } 
       
        serv_addr.sin_family = AF_INET; 
        serv_addr.sin_port = htons(PORT); 
           
        // Convert IPv4 and IPv6 addresses from text to binary form 
        if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)  
        { 
            printf("\nInvalid address/ Address not supported \n"); 
            return -1; 
        } 
       
        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
        { 
            printf("\nConnection Failed \n"); 
            return -1; 
        }
        cout <<"message from client 1 "<<i+1<<endl;  
        send(sock , hello , strlen(hello) , 0 ); 

        cout <<"message from server "<<i+1<<endl;  

        valread = read( sock , buffer, 1024); 
        printf("%s\n",buffer ); 
        cout<<"\n \n ";
        
        // sleep(200);
    }
    
    return 0; 
} 
