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


string tracker_info_path,log_file="./mytrackerLog";
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
unordered_map<string,string>userDetails;
unordered_map<string,set<string> >groupInfo;

vector<string>StringParser(string s,char del);
void process_args(char *argv[]);
fstream getLogFile();
void writeLog(string message);
void peerService(int clientSocketDes,string ip,int port);
void createNewUser(string userId,string password,int clientSocketDes);
void login(string userId,string password,int clientSocketDes);
void create_group(string userId,string groupId,int clientSocketDes);
int main(int argc, char *argv[]){

	if (argc != 3)
    {
        cout << "improper arguments" << endl;
        cout<<" please run the tracker file in this format :: ./tracker​ tracker_info.txt ​ tracker_no tracker_info.txt "<<endl;     
        exit(1);
    }
    else{
    	process_args(argv);
    	writeLog("Processed the agruments.");


    	struct sockaddr_in tracker1_address;
    	struct sockaddr_in otheraddr;
        int sock_des = 0;//socket descriptor
        int opt = 1;
        socklen_t size;
        if ((sock_des = socket(AF_INET, SOCK_STREAM, 0)) == 0)
        {
            writeLog("Socket creation error");
            exit(EXIT_FAILURE);
        }
        if (setsockopt(sock_des, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
        {
            writeLog("setsockopt Error");
            exit(EXIT_FAILURE);
        }
        tracker1_address.sin_family = AF_INET;
        tracker1_address.sin_port = htons(stoi(tracker1_port));
        tracker1_address.sin_addr.s_addr = inet_addr(tracker1_ip.c_str());

        if (bind(sock_des, (struct sockaddr *)&tracker1_address, sizeof(tracker1_address)) < 0)
        {
            writeLog("Bind Failed..");
            exit(EXIT_FAILURE);
        }
        writeLog("Bind Successful.");
        if (listen(sock_des, BACK) < 0)
        {
            writeLog("Listen Failed..");
            exit(EXIT_FAILURE);
        }
        writeLog("Socket Created with socket Descriptor = " + to_string(sock_des) + ".");
        writeLog("Listening for clients.");
        int client_socket;
        int addrlen = sizeof(tracker1_address);

        while((client_socket = accept(sock_des, (sockaddr *)&tracker1_address, (socklen_t *)&addrlen))!=-1)
		  {
		  	string ip=inet_ntoa(otheraddr.sin_addr);
		  	int port=(ntohs(otheraddr.sin_port));
		  	writeLog("Connection came from client ip "+ip +" port "+ to_string(port)+ ".");
		  	try
            {
             writeLog("Thread Created for new client.");	
		  	threadVector.push_back(thread(peerService,client_socket,ip,port));
		  	size=sizeof(struct sockaddr);
		   }
		    catch (const exception &ex)
            {
                writeLog("Thread exited with some exception. :(");
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
	    cout<<"tracker_no="<<tracker_no<<endl;	   
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

	if(userDetails.find(userId)!=userDetails.end())
	{
	  char status[]="0";
	  int s=sizeof(status);
	  send(clientSocketDes,status,s,0);
	}
	else
	{
		char status[]="1";
		userDetails[userId]=password;
		int s=sizeof(status);
	    send(clientSocketDes,status,s,0);

	}
	close(clientSocketDes);
}

void login(string userId,string password,int clientSocketDes){

	if(userDetails.find(userId)!=userDetails.end()&&(userDetails[userId]==password))
	{
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
	cout<<" Okay serve request is working "<<endl;
	
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
       login(user_id,pass,clientSocketDes);
	}
	else if(myCommand=="create_group")
	{
      string group_id=requestToServe[1];
      string user_id=requestToServe[2];
      create_group(user_id,group_id,clientSocketDes);
	}
	else if(myCommand=="join_group")
	{
      // string group_id=requestToServe[1];
      // string user_id=requestToServe[2];
      // join_group(clientSocketDes,group_id,user_id);
	}
	
	else if(myCommand=="upload_file")
	{
	  // cout<<"in line 184"<<endl;
   //    string group_id=requestToServe[1];
   //    string FileId=requestToServe[2];
   //    string IPport=requestToServe[3];
   //    cout<<"in upload "<<group_id<<" "<<FileId<<" "<<IPport<<endl;
   //    upload_file(clientSocketDes,group_id,FileId,IPport);
	}
	else if(myCommand=="share_file_details")
	{
      // string group_id=requestToServe[1];
      // string FileId=requestToServe[2];
      // string IPport=requestToServe[3];
      // share_file_details(clientSocketDes,group_id,FileId,IPport);
	}
	else if(myCommand=="seeder_list")
	{
      // string group_id=requestToServe[1];
      // string FileId=requestToServe[2];
      // //string IPport=requestToServe[3];
      // seeder_list(clientSocketDes,group_id,FileId);
	}
	else if(myCommand=="leave_group")
	{
      //  string group_id=requestToServe[1];
      // string user_id=requestToServe[2];
      // leave_group(clientSocketDes,group_id,user_id);
	}
	else if(myCommand=="list_requests")
	{
      
	}
	else if(myCommand=="list_groups")
	{
		// list_groups(clientSocketDes);

	}
	else if(myCommand=="list_files")
	 {
 //      string group_id=requestToServe[1];
 //      list_files(clientSocketDes,group_id);
	}
	
	else if(myCommand=="logout")
	{
       // string IPport=requestToServe[1];
       // string group_id=requestToServe[2];
       // logout(clientSocketDes,IPport,group_id);
	}
	


}

void create_group(string userId,string groupId,int clientSocketDes){

	if(groupInfo.find(groupId)!=groupInfo.end())
	{
	  char status[]="0";
	  send(clientSocketDes,status,sizeof(status),0);
	}
	else
	{

		char status[]="1";
		groupInfo[groupId].insert(userId);
	    send(clientSocketDes,status,sizeof(status),0);

	}
	close(clientSocketDes);
}