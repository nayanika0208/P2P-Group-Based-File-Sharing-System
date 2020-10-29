#include <stdlib.h>
#include <stdio.h>
#include<bits/stdc++.h>
#include <ctype.h>          
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <signal.h>
#include<math.h>
#include<cstring>
#include <unistd.h>
#include <openssl/sha.h>
#include <thread>
#include <semaphore.h>
#include <errno.h>
#include <string.h>


using namespace std;



string current_user;
string current_group;
bool isLoggedIn=false;

sem_t m;
string clientIPort;
string tracker_info_path,log_file="./mypeerLog";
bool startingLogFile=true;
vector<string>IP_and_port_of_Trackers;
string tracker1_ip;
string tracker2_ip;
string tracker1_port;
string tracker2_port;
string serverip;
string serverport;

mutex logfile_mutex, stastusfile_mutex;
string SEP = "|*|";
vector<thread> threadVector;
int threadCount;



vector<string>StringParser(string s,char del);
void process_args(char *argv[]);
fstream getLogFile();
void writeLog(string message);
void serverequest(int newsocketdes,string ip,int port);
void peerAsServer();
int socket_creation_to_server(string ip_address, int port_address);
void create_user(vector<string > clientRequest);
void login(vector<string > clientRequest);
void create_group(vector<string>clientRequest);



int main(int argc,char ** argv)
{
   sem_init(&m,0,1);
	if(argc!=3)
	{
		cout<<"Opps! Please give command line arguent in the format <IP>:<PORT> traker_info.txt"<<endl;
		perror("Error in command line argument list ");
		return -1;
	}
    process_args(argv);
      writeLog("Processed Arguments");
 	cout<<serverip<<" "<<serverport<<" "<<tracker1_ip<<" "<<tracker1_port<<" "<<tracker2_ip<<" "<<tracker2_port<<endl;
 	
 	thread serverthread(peerAsServer);
 	serverthread.detach();
 	    
 	
   while(true){
     string request;
   	  getline(cin,request);
      cout<<request<<endl;
      vector<string>clientRequest=StringParser(request,' ');
      string command=clientRequest[0];
      cout<<command<<endl;
   

      if(command=="create_user")
      {
   
            if(clientRequest.size()!=3)
            {
                cout<<" invalid arguments for create_user "<<endl;
                continue;
            }
            else
            {

            threadVector.push_back(thread(create_user,clientRequest));
            	
         }
      }
      else if(command=="login")
      {

            if(clientRequest.size()!=3)
            {
            	cout<<"invalid arguments for login"<<endl;
               continue;
            }
            else
            {

            	
            	threadVector.push_back(thread(login,clientRequest));

            }
      }
      else if(command=="create_group")
      {
      	
      	    if(!isLoggedIn)
       		    {
       		     	cout<<"Please first login into the system"<<endl;
       			
       		    }
            if(clientRequest.size()!=2)
            {
            	cout<<"Invalid arguments for creating group"<<endl;
               
            }
            else
            {
            	threadVector.push_back(thread(create_group,clientRequest));
            }
      }
      else if(command=="join_group")
      {
      	cout<<" Not imlemented yet " <<command<<endl;

         // cout<<"In line 343"<<endl;

         //     if(!islogedin)
         //  {
         // cout<<"Please enter the login cred to enter into the system"<<endl;
         // goto l2;
         //  }
         //    if(clientRequest.size()!=2)
         //    {
         //       cout<<"Enter the valid argument"<<endl;
         //       goto l2;
         //    }
         //    else
         //    {
         //       threadVector.push_back(thread(join_group,clientRequest[1]));
         //    }
      }
      else if(command=="leave_group")
      {
        // cout<<"In line 877"<<endl;

         //     if(!islogedin)
         //  {
         // cout<<"Please enter the login cred to enter into the system"<<endl;
         // goto l2;
         //  }
         //    if(clientRequest.size()!=2)
         //    {
         //       cout<<"Enter the valid argument"<<endl;
         //       goto l2;
         //    }
         //    else
         //    {
         //       threadVector.push_back(thread(leave_group,clientRequest[1]));
         //    }
      	cout<<" Not imlemented yet " <<command<<endl;

      }
      else if(command=="list_request")
      {
      // {
      //   cout<<"Not Inplemented "<<endl;
      //  goto l2;
      	cout<<" Not imlemented yet " <<command<<endl;

      }
      else if(command=="accept_request")
      {
       // cout<<"Not Inplemented "<<endl;
       // goto l2;
      	cout<<" Not imlemented yet " <<command<<endl;

      }
      else if(command=="list_groups")
      {
         //    cout<<"In line 358"<<endl;

         //     if(!islogedin)
         //  {
         // cout<<"Please enter the login cred to enter into the system"<<endl;
         // goto l2;
         //  }
         //  if(clientRequest.size()!=1)
         //    {
         //       cout<<"Enter the valid argument"<<endl;
         //       goto l2;
         //    }
         //    else {
         //  threadVector.push_back(thread(list_groups));

         //    }
      	cout<<" Not imlemented yet " <<command<<endl;

      }
      else if(command=="list_files")
      {
         // cout<<"In line 941 "<<endl;

         //     if(!islogedin)
         //  {
         // cout<<"Please enter the login cred to enter into the system"<<endl;
         // goto l2;
         //  }
         //    if(clientRequest.size()!=2)
         //    {
         //       cout<<"Enter the valid argument"<<endl;
         //       goto l2;
         //    }
         //    else
         //    {
         //       threadVector.push_back(thread(list_files,clientRequest[1]));
         //    }
      	cout<<" Not imlemented yet " <<command<<endl;

      }
      else if(command=="upload_file")
      {
        // cout<<"In line 697"<<endl;

        //      if(!islogedin)
        //   {
        //  cout<<"Please enter the login cred to enter into the system"<<endl;
        //  goto l2;
        //   }
        //   if(clientRequest.size()!=3)
        //     {
        //        cout<<"Enter the valid argument"<<endl;
        //        goto l2;
        //     }
        //     else {
        //        string group_id=clientRequest[1];
        //        string FileId=clientRequest[2];
        //        // cout<<group_id<<" "<<FileId<<endl;
        //   threadVector.push_back(thread(upload_file,group_id,FileId));

        //     }
        cout<<" Not imlemented yet " <<command<<endl;

      }
      else if(command=="download_file")
      {
        
            // cout<<"In line 533"<<endl;

         //     if(!islogedin)
         //  {
         // cout<<"Please enter the login cred to enter into the system"<<endl;
         // goto l2;
         //  }
         //  if(clientRequest.size()!=4)
         //    {
         //       cout<<"Enter the valid argument"<<endl;
         //       goto l2;
         //    }
         //    else {
         //       string group_id=clientRequest[1];
         //       string FileId=clientRequest[2];
         //       string Filepath=clientRequest[3];
         //  threadVector.push_back(thread(download_file,group_id,FileId,Filepath));

         //    }
      	cout<<" Not imlemented yet " <<command<<endl;

      }
      else if(command=="logout")
      {
         // cout<<"In line 894"<<endl;

         //     if(!islogedin)
         //  {
         // cout<<"Please enter the login cred to enter into the system"<<endl;
         // goto l2;
         //  }
         //    if(clientRequest.size()!=1)
         //    {
         //       cout<<"Enter the valid argument"<<endl;
         //       goto l2;
         //    }
         //    else
         //    {
         //       threadVector.push_back(thread(logout));
         //    }

      	cout<<" Not imlemented yet " <<command<<endl;

        
      }
      else if(command=="Show_downloads")
      {
            cout<<"Not Inplemented "<<endl;
       
      }
      else if(command=="stop_share")
      {
            cout<<"Not Inplemented "<<endl;
      
      }
      else if(command=="exit")
      {
        cout<<"Good bye"<<endl;
        return 0;
      }
      else
      {
      	cout<<"In line 387"<<endl;

      	// cout<<"Please Enter a valid command "<<endl;
      }
    
 	}
 	return 0;
} 


vector<string>StringParser(string s,char del)
{
  stringstream ss(s);
  vector<string>a;
  string temp;
  while(getline(ss,temp,del))
  {
  	//cout<<"hi"<<endl;
    a.push_back(temp);
  }
  return a;
}

void process_args(char *argv[])
{		string clientIPort=argv[1];
        string tracker_info_path=argv[2];	

        vector<string>IPort=StringParser(clientIPort,':');
	    serverip=IPort[0];
	    serverport=IPort[1];   
	    fstream serverfilestream(tracker_info_path,ios::in);	    
	    string temp;
	    while(getline(serverfilestream,temp,'\n'))
	    {
	    	IP_and_port_of_Trackers.push_back(temp);
	    }
	   
	    IPort=StringParser(IP_and_port_of_Trackers[0],':');
	    tracker1_ip=IPort[0];
	 	tracker1_port=IPort[1];
	 	IPort=StringParser(IP_and_port_of_Trackers[1],':');
	 	tracker2_ip=IPort[0];
	 	tracker2_port=IPort[1];
}

fstream getLogFile()
{
    logfile_mutex.lock();
    fstream my_file;
    if(startingLogFile == false){
    	 my_file.open(log_file, ios::app);
    }else {
    	my_file.open(log_file, ios::out);
    	startingLogFile = false;
    }
   
    return my_file;
}

void writeLog(string message)
{
    fstream logfile_fd;
    logfile_fd = getLogFile();
    time_t cur = time(NULL);
    string t = ctime(&cur);
    t = t.substr(4, 16);
    logfile_fd << t << ": " << message << endl;
    logfile_mutex.unlock();
    return;
}

int socket_creation_to_server(string ip_address, int port_address)
{
    struct sockaddr_in tracker1_address;
    int sock = 0;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        writeLog("Socket creation error.");
        exit(EXIT_FAILURE);
    }
    writeLog("Socket created successfully.");
    memset(&tracker1_address, '0', sizeof(tracker1_address));

    tracker1_address.sin_family = AF_INET;
    tracker1_address.sin_port = htons(port_address);

    if (inet_pton(AF_INET, ip_address.c_str(), &tracker1_address.sin_addr) <= 0)
    {
        writeLog("Invalid Tracker 1 address/ Address not supported. => " + ip_address + " is INVALID IP");
        exit(EXIT_FAILURE);
    }

    if (connect(sock, (struct sockaddr *)&tracker1_address, sizeof(tracker1_address)) < 0)
    {
        writeLog("Tracker seems Busy.. Trying to connect with Tracker 2.. ");
        return -1;
    }

    writeLog("Connected with sock =  " + to_string(sock));
    return sock;
}
void peerAsServer(){

	int sock=socket_creation_to_server(serverip,stoi(serverport));
	if(sock == -1){
		writeLog("server creation error");
	}

}

void create_user(vector<string > clientRequest){
           int s_des=socket_creation_to_server(tracker1_ip,stoi(tracker1_port));
           string token="create_user;"+clientRequest[1]+";"+clientRequest[2];
           // token+=";";
           // token+=clientRequest[1];
           // token+=";";
           // token+=clientRequest[2];
           send(s_des,token.c_str(),strlen(token.c_str()),0);
            char status[]={0};
            int valRead=read( s_des , status, sizeof(status));
            if(status[0]=='0'){
              cout<<"User Already exists"<<endl;

            
            }else{
              cout<<" User Created"<<endl;
            }
            
            
}

void login(vector<string > clientRequest)
{
   int s_des=socket_creation_to_server(tracker1_ip,stoi(tracker1_port));
   string token="login;"+clientRequest[1]+";"+clientRequest[2];
   send(s_des,token.c_str(),strlen(token.c_str()),0);


   char status[]={0};
    int valRead=read( s_des , status, sizeof(status));
    if(status[0]=='1'){
       
           isLoggedIn=true;
          current_user=clientRequest[1];
          cout<<clientRequest[1]<<" Logged in successfully"<<endl;
          writeLog(clientRequest[1]+"Logged in successfully");
        }else{
         cout<<"Wrong Credentials"<<endl;
         writeLog("Login failed"); 
     }
}

void create_group(vector<string>clientRequest)
{
   int s_des=socket_creation_to_server(tracker1_ip,stoi(tracker1_port));
   string token="create_group;"+clientRequest[1]+";"+current_user;
   send(s_des,token.c_str(),strlen(token.c_str()),0);
   
   char status[]={0};
   int valRead=read( s_des , status, sizeof(status));
   if(status[0]=='1')
   {
    cout<<clientRequest[1]<<"Group created succesfully"<<endl;
    writeLog(current_user+" created "+ clientRequest[1]+" group succesfully ");
   }
   else
   {
    cout<<"Group already exits"<<endl;
    cout<<"Try creating another group again"<<endl;
   }
  
}
