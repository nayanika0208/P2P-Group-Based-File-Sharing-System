#include <bits/stdc++.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <unistd.h>
#include <openssl/sha.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <ctime>

using namespace std;

#define BUFFER_SIZE 10240
#define BACK 2500

struct userInfo{
  string userId;
  string password;
  string currentIp;
  int portno;
  bool isLoggedIn=false;
};

struct groupInfo{

   string group_id;
   string ownwerId;

};


struct file_info
{
  string fileName; //assuming file name will be unique
   unsigned long long filesize;
  string fullFileSha;
  vector<string> chunkWiseSha;
  int noOfChunks;
};

struct seederInfo{
  string seederName;
  string fileName;
  string filePath;

};



unordered_map<string,struct userInfo >userToUserInfo;
unordered_map<string,struct file_info>fileToFIleInfo;
unordered_map<string,struct groupInfo >GroupToGroupInfo;

unordered_map<string,set<string> >pendingInvites;
unordered_map<string,set<string> >usersIngroup;
unordered_map<string,set<string> >FilesInGroup;
unordered_map< string,set<string > > seederList;//file name to users
// unordered_map< string,vector<seederInfo  > > seederListInfo;//file name to seeder info

map< pair<string,string> ,string> seederFile_to_path;
vector<string >show_downloads;

string tracker_info_path,log_file="./mytrackerLog1";
bool startingLogFile=true;
int tracker_no;
vector<string>IP_and_port_of_Trackers;
string tracker1_ip;
string tracker2_ip;
string tracker1_port;
string tracker2_port;
mutex seedfile_mutex, logfile_mutex;
string SEP = "|*|";
vector<thread> threadVector;
int thread_count;


vector<string>StringParser(string s,char del);
void process_args(char *argv[]);
fstream getLogFile();
void writeLog(string message);
void peerService(int clientSocketDes,string ip,int port);
void createNewUser(string userId,string password,int clientSocketDes);
void login(string userId,string password,string userip,int userport,int clientSocketDes);
void createGroup(string userId,string groupId,int clientSocketDes);
void join_group(string groupId,string userId,int clientSocketDes);
void  leave_group(string groupId,string userId,int clientSocketDes);
void  list_groups(int clientSocketDes);
void  accept_requests(int clientSocketDes,string groupId,string userId,string currentUser);
void  list_requests(int clientSocketDes,string groupId,string currentUser);
void list_files(int clientSocketDes,string group_id);
void getSeeders(int clientSocketDes,string file_id,string group_id,string curr);


int main(int argc, char *argv[]){

  if (argc != 3)
    {
        cout << "improper arguments" << endl;
        cout<<" please run the tracker file in this format :: ./tracker​ tracker_info.txt ​ tracker_no tracker_info.txt "<<endl;     
        exit(1);
    }
    else{
      cout<<" came here1 "<<endl;
      process_args(argv);
      // writeLog("Processed the agruments.");


      struct sockaddr_in tracker1_address;
      struct sockaddr_in otheraddr;
        int sock_des = 0;//socket descriptor
        int opt = 1;
        socklen_t size;
        if ((sock_des = socket(AF_INET, SOCK_STREAM, 0)) == 0)
        {
            // writeLog("Socket creation error");
            perror("failed to obtained the socket descriptor");
            exit(EXIT_FAILURE);
        }
        if (setsockopt(sock_des, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
        {
            // writeLog("setsockopt Error");
            exit(EXIT_FAILURE);
        }
        tracker1_address.sin_family = AF_INET;
        tracker1_address.sin_port = htons(stoi(tracker1_port));
        tracker1_address.sin_addr.s_addr = inet_addr(tracker1_ip.c_str());

        if (bind(sock_des, (struct sockaddr *)&tracker1_address, sizeof(tracker1_address)) < 0)
        {
           perror("failed to obtained the bind");
            exit(EXIT_FAILURE);
        }
        // writeLog("Bind Successful.");
        if (listen(sock_des, BACK) < 0)
        {
            perror("error backlog overflow");
            exit(EXIT_FAILURE);
        }
        // writeLog("Socket Created with socket Descriptor = " + to_string(sock_des) + ".");
        // writeLog("Listening for clients.");
        int client_socket;
        int addrlen = sizeof(tracker1_address);

        while((client_socket = accept(sock_des, (sockaddr *)&tracker1_address, (socklen_t *)&addrlen))!=-1)
      {
        string ip=inet_ntoa(otheraddr.sin_addr);
        int port=(ntohs(otheraddr.sin_port));
        cout<<" Serving request from client"<<endl;
        // cout<<"Request came from client ip "<<otheraddr.sin_addr<<" port "<<otheraddr.sin_port<<endl;
        // writeLog("Connection came from client ip "+ip +" port "+ to_string(port)+ ".");
        try
            {
             // writeLog("Thread Created for new client."); 
        threadVector.push_back(thread(peerService,client_socket,ip,port));
        size=sizeof(struct sockaddr);
       }
        catch (const exception &ex)
            {
                // writeLog("Thread exited with some exception. :(");
            }
      }
      vector<thread>:: iterator it;
      for(it=threadVector.begin();it!=threadVector.end();it++)

      {
        if(it->joinable()) 
          it->join();
        }
    cout<<"retruning form Tracker main"<<endl;
       
    }
}

vector<string>StringParser(string s,char del)
{
  //cout<<"my string parser"<<endl;
  stringstream ss(s);
  vector<string>a;
  string temp;
  while(getline(ss,temp,del))
  {
    //cout<<"parsing string"<<endl;
    a.push_back(temp);
  }
  return a;
}

void process_args(char *argv[])
{
      
        string tracker_info_path=argv[1];
      tracker_no=stoi(argv[2]);
      cout<<"Tracker_no="<<tracker_no<<endl;     
      fstream serverfilestream(tracker_info_path,ios::in);
       
      string temp;
      while(getline(serverfilestream,temp,'\n'))
      {
        IP_and_port_of_Trackers.push_back(temp);
        
      }
      vector<string>IPort;
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

void createNewUser(string userId,string password,int clientSocketDes){

  if(userToUserInfo.find(userId)!=userToUserInfo.end())
  {
    char status[]="0";
    int s=sizeof(status);
    send(clientSocketDes,status,s,0);
  }
  else
  {
    char status[]="1";
    userInfo user;
    user.userId=userId;
    user.password=password;
    userToUserInfo[userId]=user;
    // cout<<userToUserInfo[userId].userId<<endl;
    // cout<<userToUserInfo[userId].password<<endl;
    int s=sizeof(status);
      send(clientSocketDes,status,s,0);

  }
  close(clientSocketDes);
}

void login(string userId,string password,string userip,int userport,int clientSocketDes){

  if(userToUserInfo.find(userId)!=userToUserInfo.end()&&(userToUserInfo[userId].password==password))
  {
    cout<<" loging in "<<userId <<endl;
    userToUserInfo[userId].currentIp=userip;
     userToUserInfo[userId].portno=userport;
     userToUserInfo[userId].isLoggedIn=true;
    char  status[]="1";
    
    send(clientSocketDes,status,sizeof(status),0);
  }
  else
  {
    char  status[]="0";
      send(clientSocketDes,status,sizeof(status),0);

  }
  close(clientSocketDes);
  
}

void peerService(int clientSocketDes,string ip,int port)
{
  // cout<<" Okay serve request is working "<<endl;
  
        char buffer[BUFFER_SIZE] = {0}; 
        int valread = read( clientSocketDes , buffer, BUFFER_SIZE); 
        string request=buffer;
        vector<string> requestToServe=StringParser(request,';');
        string myCommand= requestToServe[0];

  if(myCommand=="create_user")
  {
     string user_id=requestToServe[1];
     string pass=requestToServe[2];
       createNewUser(user_id,pass,clientSocketDes);
  }
  else if(myCommand=="login")
  {
       string user_id=requestToServe[1];
       string pass=requestToServe[2];
       string userip=requestToServe[3];
       int userport=stoi(requestToServe[4]);
       login(user_id,pass,userip,userport,clientSocketDes);
  }
  else if(myCommand=="create_group")
  {
      string group_id=requestToServe[1];
      string user_id=requestToServe[2];
      createGroup(user_id,group_id,clientSocketDes);
  }
  else if(myCommand=="join_group")
  {
      string group_id=requestToServe[1];
      string user_id=requestToServe[2];
      join_group(group_id,user_id,clientSocketDes);
  }
  
  else if(myCommand=="upload_file")
  {


    string info="";
     string group_id=requestToServe[3];
      string user_id=requestToServe[4]; 
       string FileId=requestToServe[2];


      if(GroupToGroupInfo.find(group_id)!=GroupToGroupInfo.end()){
        if(usersIngroup[group_id].find(user_id)!= usersIngroup[group_id].end()){
          if(FilesInGroup[group_id].find(FileId)==FilesInGroup[group_id].end()){
            FilesInGroup[group_id].insert(FileId);
          }
          seederList[FileId].insert(user_id);
         
       file_info f_info;
         f_info.fileName=FileId;
         f_info.filesize=stoull(requestToServe[5]);
         f_info.fullFileSha=requestToServe[7];
         f_info.noOfChunks=stoi(requestToServe[6]);
         for(int i=8;i<requestToServe.size();i++){
            f_info.chunkWiseSha.push_back(requestToServe[i]);
          }
          
          fileToFIleInfo[requestToServe[2]]=f_info;

          seederFile_to_path[make_pair(user_id,FileId)]=requestToServe[1];
          // cout<<" file seeder to file path "<<endl;
          cout<<seederFile_to_path[make_pair(user_id,FileId)]<<endl;
         
           cout<<"Uploading "<<group_id<<" "<<FileId<<" "<<userToUserInfo[user_id].portno<<endl;

        }else{
          info+="User does not belong to the group";
    send(clientSocketDes,info.c_str(),strlen(info.c_str()),0);
        }
      }else{
    info+="the group does not exist";
    send(clientSocketDes,info.c_str(),strlen(info.c_str()),0);
      }
     
      
     
     
  }
  
  else if(myCommand=="add_to_seeder_list")
  {


      string group_id=requestToServe[1];
      string FileId=requestToServe[2];
       string FilePath=requestToServe[3];
       string user_id=requestToServe[4];
       seederList[FileId].insert(user_id);
        cout<<"adding user to seeder list "<<user_id<<endl;
        string info="C "+group_id+" "+FileId;
        // cout<<" info "<<info<<endl;
        show_downloads.push_back(info);


       seederFile_to_path[make_pair(user_id,FileId)]=FilePath;
  }
  else if(myCommand=="leave_group")
  {
      string group_id=requestToServe[1];
      string user_id=requestToServe[2];
      leave_group(group_id,user_id,clientSocketDes);
  }
  else if(myCommand=="list_requests")
  {
     string group_id=requestToServe[1];
      string curr=requestToServe[2];
     list_requests(clientSocketDes,group_id,curr);

      
  }
  else if(myCommand=="get_active_seeders")
  {
    
    string currentUser=requestToServe[3];
    
     string group_id=requestToServe[2];
    
      string file_id=requestToServe[1];
     getSeeders(clientSocketDes,file_id,group_id,currentUser);

      
  }

  
  else if(myCommand=="list_groups")
  {
    list_groups(clientSocketDes);

  }
  else if(myCommand=="list_files")
  {
     string group_id=requestToServe[1];
    list_files(clientSocketDes,group_id);

  }
  else if(myCommand=="accept_request")
   {
      string group_id=requestToServe[1];
      string user_id=requestToServe[2];
      string curr=requestToServe[3];
      accept_requests(clientSocketDes,group_id,user_id,curr);
  }
  
  else if(myCommand=="logout")
  {
      
      string userId=requestToServe[1];
      userToUserInfo[userId].isLoggedIn=false;

  }
  else if(myCommand=="show_downloads")
  {
      
    
      string info="";
      if(show_downloads.size()>0){
         for(int i=0;i<show_downloads.size()-1;i++){
        info+=show_downloads[i];
        info+=";";
      }
      info+=show_downloads[show_downloads.size()-1];

      send(clientSocketDes,info.c_str(),info.size(),0);
      }
     

  }
  


}

void createGroup(string userId,string groupId,int clientSocketDes){

  if(GroupToGroupInfo.find(groupId)!=GroupToGroupInfo.end())
  {
    char status[]="0";
    send(clientSocketDes,status,sizeof(status),0);
  }
  else
  {

    char status[]="1";
    groupInfo grp;
    grp.group_id=groupId;
    grp.ownwerId=userId;
    GroupToGroupInfo[groupId]=grp;
    // cout<<GroupToGroupInfo[groupId].group_id<<"  "<<GroupToGroupInfo[groupId].ownwerId<<endl;
    usersIngroup[groupId].insert(userId);

      send(clientSocketDes,status,sizeof(status),0);

  }
  close(clientSocketDes);
}

void join_group(string groupId,string userId,int clientSocketDes){

    if(usersIngroup.find(groupId)!=usersIngroup.end())
  {
    
     if(usersIngroup[groupId].find(userId)!=usersIngroup[groupId].end()){
      char status[]="2";
      send(clientSocketDes,status,sizeof(status),0);
     }else{
      pendingInvites[groupId].insert(userId);
      char status[]="1";
      send(clientSocketDes,status,sizeof(status),0);
     }      

  }
  else
  {

    char status[]="0";
    send(clientSocketDes,status,sizeof(status),0);

  }
  close(clientSocketDes);
  
  
}

void  leave_group(string groupId,string userId,int clientSocketDes)
{
  
    if(GroupToGroupInfo.find(groupId)!=GroupToGroupInfo.end())
  {   
    if(GroupToGroupInfo[groupId].ownwerId == userId){
      GroupToGroupInfo.erase(groupId);
      pendingInvites.erase(groupId);
             usersIngroup.erase(groupId);
             FilesInGroup.erase(groupId);
             char status[]="1";
             send(clientSocketDes,status,sizeof(status),0);

    }else{
       if(usersIngroup[groupId].find(userId)!=usersIngroup[groupId].end()){
      char status[]="1";
      usersIngroup[groupId].erase(userId);
      send(clientSocketDes,status,sizeof(status),0);
     } 
    }
    
      

  }
  else
  {

    char status[]="0";
    send(clientSocketDes,status,sizeof(status),0);

  }
  close(clientSocketDes);

  
}

void  list_groups(int clientSocketDes)
{
  string info="";
  for(auto &i:GroupToGroupInfo)
  {
      info+=i.first;
      info+=";";
  }
  send(clientSocketDes,info.c_str(),strlen(info.c_str()),0);
  close(clientSocketDes);
  
}
void  accept_requests(int clientSocketDes,string groupId,string userId,string currentUser)
{

  if(currentUser != GroupToGroupInfo[groupId].ownwerId){
    string info="You are not the owner of this group ";
    send(clientSocketDes,info.c_str(),strlen(info.c_str()),0);
  }else{
    pendingInvites[groupId].erase(userId);
    usersIngroup[groupId].insert(userId);
    string info=userId+" is now a part of group "+groupId;
    send(clientSocketDes,info.c_str(),strlen(info.c_str()),0);
  }
  
  
  close(clientSocketDes);
  
}
void  list_requests(int clientSocketDes,string groupId,string currentUser)
{
  if(currentUser != GroupToGroupInfo[groupId].ownwerId){
    string info="You are not the owner of this group ";
    send(clientSocketDes,info.c_str(),strlen(info.c_str()),0);
  }else{
    string info="";
    if(pendingInvites.find(groupId) == pendingInvites.end()){
      send(clientSocketDes,info.c_str(),strlen(info.c_str()),0);
    }else{
      for(auto i:pendingInvites[groupId])
      {
          info+= i;
          info+=";";
        
      }
      send(clientSocketDes,info.c_str(),strlen(info.c_str()),0);
    
    
    
    }

  }
  close(clientSocketDes);
  
}
void list_files(int clientSocketDes,string group_id){
 
  string info="";
  for(auto i:FilesInGroup[group_id])
   {
          info+= i;
          info+=";";
        
    }
  send(clientSocketDes,info.c_str(),strlen(info.c_str()),0);

}
void getSeeders(int clientSocketDes,string file_id,string group_id,string curr){
 // cout<<" getting seeders "<<endl;
  string info="";
  char status[]={0};

  //valid group
  //check user in group
  //check file in group

  if(GroupToGroupInfo.find(group_id) == GroupToGroupInfo.end()){
    cout<<"Invalid group"<<endl;
    send(clientSocketDes,status,1,0);
  }else{
    if(usersIngroup[group_id].find(curr) == usersIngroup[group_id].end()){
      cout<<"Invalid user "<<endl;
      send(clientSocketDes,status,1,0);
    }else {
      if(FilesInGroup[group_id].find(file_id) == FilesInGroup[group_id].end()){
        cout<<"No such fiile "<<endl;
        send(clientSocketDes,status,1,0);
      }else{

        status[0]='1';
        // cout<<" sending ports of active seeders "<<endl;
        send(clientSocketDes,status,sizeof(status),0);
        // cout<<"size "<<seederList[file_id].size()<<endl;

        vector<string> seeders_name;
        for(auto i:seederList[file_id]){
          seeders_name.push_back(i);
          // cout<<i<<endl;
        }
        vector <string> ip_port_active;
        info+=to_string(seeders_name.size());
        info+=";";
        for(int i=0;i<seeders_name.size();i++){
          if(userToUserInfo[seeders_name[i]].isLoggedIn == true){
            string ip=userToUserInfo[seeders_name[i]].currentIp;
          int port=userToUserInfo[seeders_name[i]].portno;
          
          string seedpath=seederFile_to_path[make_pair(seeders_name[i],file_id)];
          string ip_port=ip+":"+to_string(port)+":"+seeders_name[i]+":"+seedpath;
          ip_port_active.push_back(ip_port);
          }
          
          
        }
        for(int i=0;i<ip_port_active.size()-1;i++){
          
          info+=ip_port_active[i];
          info+=";";
          
          
        }
        info+=ip_port_active[ip_port_active.size()-1];
          
      send(clientSocketDes,info.c_str(),strlen(info.c_str()),0);

      info="";
      file_info file=fileToFIleInfo[file_id];
      info+=to_string(file.filesize);
      info+=";";
      info+=file.fullFileSha;
      info+=";";
      info+=to_string(file.noOfChunks);
      info+=";";

      for(int i=0;i<file.noOfChunks-1;i++){
        info+=file.chunkWiseSha[i];
        info+=";";
      }
      info+=file.chunkWiseSha[file.noOfChunks-1];
      // cout<<info<<endl;
      send(clientSocketDes,info.c_str(),strlen(info.c_str()),0);

      }
    }
  }
  
}

