#include <unistd.h>
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
#include <openssl/sha.h>
#include <thread>
#include <semaphore.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <pwd.h>
#include <grp.h>


using namespace std;


#define BUFFER_SIZE 10240
#define BACK 2500

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
void join_group(vector<string>clientRequest);
void list_requests(vector<string > clientRequest);
void accept_request(vector<string>clientRequest);
void list_files(vector<string>clientRequest);
void leave_group(vector<string>clientRequest);
size_t get_file_size(string f_name);
void list_groups();
int isFileExist(string path);
void upload_file(vector<string>clientRequest);
string get_SHA1(char *data, int chnk_size);


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
      

        
             if(!isLoggedIn)
              {
                cout<<"Please first login into the system"<<endl;
                continue;
            
              }
            if(clientRequest.size()!=2)
            {
              cout<<"Invalid arguments for joining group"<<endl;
               
            }
            else
            {
               threadVector.push_back(thread(join_group,clientRequest));
            }
      }
      else if(command=="leave_group")
      {
       

             if(!isLoggedIn)
          {
               cout<<"Please first login into the system"<<endl;
               continue;
                   }
            if(clientRequest.size()!=2)
            {
               cout<<"Invalid arguments for joining group"<<endl;
               
            }
            else
            {
               threadVector.push_back(thread(leave_group,clientRequest));
            }
      	

      }
      else if(command=="requests")
      {
      
      	 if(!isLoggedIn)
          {
               cout<<"Please first login into the system"<<endl;
               continue;
                   }
            if(clientRequest.size()!=3)
            {
               cout<<"Invalid arguments for listing requets"<<endl;
               
            }
            else
            {
               threadVector.push_back(thread(list_requests,clientRequest));
            }

      }
      else if(command=="accept_request")
      {
         if(!isLoggedIn)
          {
               cout<<"Please first login into the system"<<endl;
               continue;
                   }
       if(clientRequest.size()!=3)
            {
               cout<<"Enter the valid argument"<<endl;
              
            }
            else {
          threadVector.push_back(thread(accept_request,clientRequest));

           }

      }
      else if(command=="list_groups")
      {
        if(!isLoggedIn)
          {
               cout<<"Please first login into the system"<<endl;
               continue;
                   }
            
          if(clientRequest.size()!=1)
            {
               cout<<"Enter the valid argument"<<endl;
              
            }
            else {
          threadVector.push_back(thread(list_groups));

           }
      	
      }
      else if(command=="list_files")
      {
         if(!isLoggedIn)
          {
               cout<<"Please first login into the system"<<endl;
               continue;
            }
          
            if(clientRequest.size()!=2)
            {
               cout<<"Enter the valid argument"<<endl;
              
            }
            else
            {
               threadVector.push_back(thread(list_files,clientRequest));
            }
      	

      }
      else if(command=="upload_file")
      {
       

         if(!isLoggedIn)
          {
               cout<<"Please first login into the system"<<endl;
               continue;
         }
          if(clientRequest.size()!=3)
            {
               cout<<"Enter the valid argument"<<endl;
              
            }
            else {
             
          threadVector.push_back(thread(upload_file,clientRequest));

        

      }
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
   string token="login;"+clientRequest[1]+";"+clientRequest[2]+";"+serverip+";"+serverport;
   cout<<token<<endl;
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
void list_requests(vector<string > clientRequest)
{
   int s_des=socket_creation_to_server(tracker1_ip,stoi(tracker1_port));
   string token="list_requests;"+clientRequest[2]+";"+current_user;
   
   send(s_des,token.c_str(),strlen(token.c_str()),0);


  
char buffer[BUFFER_SIZE]={0};
   int valRead=read( s_des ,buffer, sizeof(buffer));
   string buff(buffer);
   vector<string>req=StringParser(buff,';');
   if(req.size() == 0){
    cout<<" No requests in group"<<endl;
   }else{
    cout<<"Requests in group :"<<endl;
    for(int i=0;i<req.size();i++){
      cout<<req[i]<<endl;
    }
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

void join_group(vector<string>clientRequest)
{
   int s_des=socket_creation_to_server(tracker1_ip,stoi(tracker1_port));
   string token="join_group;"+clientRequest[1]+";"+current_user;
   send(s_des,token.c_str(),strlen(token.c_str()),0);
   
   char status[]={0};
   int valRead=read( s_des , status, sizeof(status));
   if(status[0]=='1')
   {
    cout<<current_user<<" requested to join  "<< clientRequest[1]<<endl;
    writeLog(current_user+" requested to join  "+ clientRequest[1]);
   }else if(status[0] == '2'){
    cout<<current_user<<"is already part Group "<<clientRequest[1]<<endl;

   }

   
   else
   {
    cout<<"Could not join the group!! Try again with valid group"<<endl;

  }
  
}

void leave_group(vector<string>clientRequest)
{
   int s_des=socket_creation_to_server(tracker1_ip,stoi(tracker1_port));
   string token="leave_group;"+clientRequest[1]+";"+current_user;
   send(s_des,token.c_str(),strlen(token.c_str()),0);
   
   char status[]={0};
   int valRead=read( s_des , status, sizeof(status));
   if(status[0]=='1')
   {
    cout<<current_user<<"  Left "<< clientRequest[1]<<"  group succesfully"<<endl;
    writeLog(current_user+" left "+ clientRequest[1]+" group succesfully ");
   }
   else
   {
    cout<<"Could not leave the group!! Try again with valid group"<<endl;

  }
  
}

void list_groups()
{
  cout<<" list group is working" <<endl;
   int s_des=socket_creation_to_server(tracker1_ip,stoi(tracker1_port));
   string token="list_groups;";
   send(s_des,token.c_str(),strlen(token.c_str()),0);
   
   char buffer[BUFFER_SIZE]={0};
   int valRead=read( s_des ,buffer, sizeof(buffer));
   string buff(buffer);
   vector<string>grouplist=StringParser(buff,';');
   if(grouplist.size() == 0){
    cout<<" No groups in the network "<<endl;
   }else{
    cout<<"Groups in network are :"<<endl;
    for(int i=0;i<grouplist.size();i++){
      cout<<grouplist[i]<<endl;
    }
   }
  


}

void accept_request(vector<string>clientRequest)
{
  
   int s_des=socket_creation_to_server(tracker1_ip,stoi(tracker1_port));
   string token="accept_request;"+clientRequest[1]+";"+clientRequest[2]+";"+current_user;
   send(s_des,token.c_str(),strlen(token.c_str()),0);
   
   char buffer[BUFFER_SIZE]={0};
   int valRead=read( s_des ,buffer, sizeof(buffer));
   string buff(buffer);
   cout<<buffer<<endl;

  
}

void list_files(vector<string>clientRequest)
{
  
   int s_des=socket_creation_to_server(tracker1_ip,stoi(tracker1_port));
   string token="list_files;"+clientRequest[1];
   send(s_des,token.c_str(),strlen(token.c_str()),0);
   
   char buffer[BUFFER_SIZE]={0};
   int valRead=read( s_des ,buffer, sizeof(buffer));
    string buff(buffer);
   vector<string>fileList=StringParser(buff,';');
  
    if(fileList.size() == 0){
    cout<<" No file in the group "<<endl;
   }else{
    cout<<"Files in the group are:"<<endl;
    for(int i=0;i<fileList.size();i++){
      cout<<fileList[i]<<endl;
    }
   }

  
}

size_t get_file_size(string f_name)
{
    struct stat sb;
    int rc = stat(f_name.c_str(), &sb);
    return rc == 0 ? sb.st_size : -1;
}

string get_SHA1(char *data, int chnk_size)
{
    unsigned char md[20];
    unsigned char buf[40];
    SHA1((unsigned char *)data, chnk_size, md);
    for (int i = 0; i < 20; i++)
    {
        sprintf((char *)&(buf[i * 2]), "%02x", md[i]);
    }
    string md_1((char *)buf);
    return md_1;
}


// void mtorrent_generator(string f_name, string tor_name)
// {
//     string t_ip1 = tr1_ip;
//     int t_port1 = tr1_port;
//     string t_ip2 = tr2_ip;
//     int t_port2 = tr2_port;

//     ofstream mtorrent_file;
//     ifstream input_file;

//     input_file.open(f_name, ios::binary | ios::in);
//     mtorrent_file.open(tor_name);


//     mtorrent_file << t_ip1 << ":" << t_port1 << endl;
//     mtorrent_file << t_ip2 << ":" << t_port2 << endl;
//     mtorrent_file << f_name << endl; // CREATE FILENAME TO ABSOLUTE PATH
//     mtorrent_file << f_size << endl;

//     size_t chnk_size = 512 * 1024; //512KB

//     unsigned long long s = f_size;
//     if (s < chnk_size)
//         chnk_size = s;
//     char tmp[chnk_size];
//     string final_hash;
//     writeLog("Reading " + tor_name + " and creating mtorrent file.");
//     while (input_file.read(tmp, chnk_size))
//     {
//         s -= chnk_size;
//         string md_1 = get_SHA1(tmp, chnk_size);
//         final_hash.append(md_1.substr(0, 20));
//         if (s == 0)
//             break;
//         if (s < chnk_size)
//             chnk_size = s;
//     }
//     mtorrent_file << final_hash << endl;

//     writeLog(tor_name + " generated.");
// }
void upload_file(vector<string>clientRequest){
  int s_des=socket_creation_to_server(tracker1_ip,stoi(tracker1_port));

  string filepath=clientRequest[1];
 
  if(isFileExist(filepath)){
      ifstream input_file;

    size_t f_size = get_file_size(filepath);
    input_file.open(filepath, ios::binary | ios::in);
    size_t chnk_size = 512 * 1024; //512KB

    vector<string>chunkHash;
    unsigned long long chunks=0;
    unsigned long long s = f_size;
    if (s < chnk_size){
       chnk_size = s;
   
    }
       
    char tmp[chnk_size];
    string final_hash;
   
    while (input_file.read(tmp, chnk_size))
    {
        s -= chnk_size;
        chunks+=1;
        string md_1 = get_SHA1(tmp, chnk_size);
        final_hash.append(md_1.substr(0, 20));
        chunkHash.push_back(md_1.substr(0, 20));
        if (s == 0)
            break;
        if (s < chnk_size)
            chnk_size = s;
    }

    cout<<" final hash "<<final_hash<<endl;
    for(int i=0;i<chunkHash.size();i++){
      cout<<chunkHash[i]<<endl;
    }

    string token="upload_file;"+filepath+";"+clientRequest[2]+";"+current_user+";"+to_string(f_size)+";"+to_string(chunks)+";"+final_hash+";";

    for(int i=0;i<chunkHash.size()-1;i++){
      token+=chunkHash[i]+";";
    }
    token+=chunkHash[chunkHash.size()-1];
    cout<<"token  "<<token<<endl;
     send(s_des,token.c_str(),strlen(token.c_str()),0);
   }else{
    cout<<" The mentioned file does not exist "<<endl;
   }
    


    



}

int isFileExist(string path)
{
    FILE *file;
    if ((file = fopen(path.c_str(), "r")))
    {
        fclose(file);
        return 1;
    }

    return 0;
}
