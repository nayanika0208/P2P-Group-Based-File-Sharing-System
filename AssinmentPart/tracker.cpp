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



void append_to_seederlist(string);
void write_to_seederlist();
void process_args(char **);
void read_seederlist();
void print_map();
void writeLog(string);
fstream getSeederListFile(int mode);
void serve(int cl_soc);





string tr1_ip, tr2_ip, seeder_list, log_file;
int tr1_port, tr2_port;
map<string, map<string, string>> seeder_map;
mutex seedfile_mutex, logfile_mutex;
string SEP = "|*|";

vector<thread> threadVector;
int threadCount;
map<string,string> user_info; //user id ,password
map<string,set<string> >GroupInfo;

map<pair<string,string>,set<string> >seederlist;

map<pair<string,string>,int > FileSizemap;
map<string,string>FileMap;


map<string,set<string> >GroupAndFile;

vector<string>ArrayOfString(string s,char del);
void  create_user(int newsocketdes,string user_id,string pass);

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        cout << "improper arguments :( \n"
             << endl;
        cout<<" please run the tracker file in this format :: ./tracker​ tracker_info.txt ​ tracker_no tracker_info.txt "<<endl;     
        exit(1);
    }
    else
    {
        process_args(argv);
	    string traker_info_path=argv[1];
	    int tracker_no=stoi(argv[2]);
	    cout<<"tracker_no="<<tracker_no<<endl;
	    char buffer[BUFF_SIZE];

	   
	    fstream serverfilestream(traker_info_path,ios::in);
	    vector<string>IPortTrakers;
	    string temp;
	    while(getline(serverfilestream,temp,'\n'))
	    {
	    	IPortTrakers.push_back(temp);
	    }
	    vector<string>IPort;
	    IPort=ArrayOfString(IPortTrakers[0],':');
	    TR1ip=IPort[0];
	 	TR1port=IPort[1];
	 	IPort=ArrayOfString(IPortTrakers[1],':');
	 	TR2ip=IPort[0];
	 	TR2port=IPort[1];
        writeLog("Argument Processed.");
        read_seederlist();
        writeLog("SeededList File Read Successfully.");
        struct sockaddr_in tr1_addr;
        int sock = 0;
        int opt = 1;
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == 0)
        {
            writeLog("Socket creation error");
            exit(EXIT_FAILURE);
        }
        if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
        {
            writeLog("setsockopt Error");
            exit(EXIT_FAILURE);
        }
        tr1_addr.sin_family = AF_INET;
        tr1_addr.sin_port = htons(tr1_port);
        tr1_addr.sin_addr.s_addr = inet_addr(tr1_ip.c_str());

        if (bind(sock, (struct sockaddr *)&tr1_addr, sizeof(tr1_addr)) < 0)
        {
            writeLog("Bind Failed..");
            exit(EXIT_FAILURE);
        }
        writeLog("Bind Successful.");
        if (listen(sock, 5) < 0)
        {
            writeLog("Listen Failed..");
            exit(EXIT_FAILURE);
        }
        writeLog("Socket Created with sock = " + to_string(sock) + ".");
        writeLog("Listening for clients.");
        int cl_soc;
        int addrlen = sizeof(tr1_addr);
        while (1)
        {
            writeLog("Waiting for client :");
            cl_soc = accept(sock, (struct sockaddr *)&tr1_addr, (socklen_t *)&addrlen);
            if (cl_soc < 0)
            {
                writeLog("in accept, something unwanted occured...");
                continue;
            }
            writeLog("Connection accepted.");
            try
            {
                writeLog("Thread Created for new client.");
                std::thread t1(serve, std::ref(cl_soc));
                t1.detach();
            }
            catch (const std::exception &ex)
            {
                writeLog("Thread exited with some exception. :(");
            }
        }
    }
    return 0;
}




/*
 * give protected fstream object of logfile to 
 * write log in logfile.
 */
fstream getLogFile()
{
    logfile_mutex.lock();
    fstream my_file;
    my_file.open(log_file, ios::app);
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

/*
 * give protected fstream object of seeder 
 * file to write and read from it.
 */
fstream getSeederListFile(int mode)
{
    seedfile_mutex.lock();
    fstream my_file;
    if (mode == 0)
        my_file.open(seeder_list, ios::in);
    if (mode == 1)
        my_file.open(seeder_list, ios::out);
    if (mode == 2)
        my_file.open(seeder_list, ios::app);
    return my_file;
}

/*
 * APPEND data to seeder file 
 */
void append_to_seederlist(string data)
{
    fstream seeder_file;
    seeder_file = getSeederListFile(2);
    seeder_file << data << endl;
    seeder_file.close();
    seedfile_mutex.unlock();
    writeLog("SeederList file updated.");
    return;
}

/*
 * take file stream object of seeder file and overwrite whole map to 
 * that file into it. 
 */
void write_to_seederlist()
{
    fstream seed_file;
    seed_file = getSeederListFile(1);
    string s;
    for (auto i : seeder_map)
    {
        for (auto j : i.second)
            seed_file << i.first << SEP << j.first << SEP << j.second << endl;
    }
    seed_file.close();
    seedfile_mutex.unlock();
    writeLog("SeederList file updated.");
    return;
}

/*
 * At Wakeup time, read seederlist file and 
 * dump that content to map
 */
void read_seederlist()
{
    fstream seed_file;
    seed_file = getSeederListFile(0);
    string s;
    while (getline(seed_file, s))
    {
        char *token = strtok((char *)s.c_str(), SEP.c_str());
        string key_hash = token;
        token = strtok(NULL, SEP.c_str());
        string cl_socket = token;
        token = strtok(NULL, SEP.c_str());
        string file_path = token;
        seeder_map[key_hash][cl_socket] = file_path;
    }
    seed_file.close();
    seedfile_mutex.unlock();
    // print_map();
    return;
}

/*
 * Process command line arguments and stores
 * all ip/port in apropriate fields in global
 * variables.
 */
void process_args(char *argv[])
{
    char *token = strtok(argv[1], ":");
    tr1_ip = token;
    token = strtok(NULL, ":");
    tr1_port = stoi(token);

    token = strtok(argv[2], ":");
    tr2_ip = token;
    token = strtok(NULL, ":");
    tr2_port = stoi(token);

    seeder_list = argv[3];
    log_file = argv[4];
}

/*
 * prints whole content of map and print on screen
 */
void print_map()
{
    if (seeder_map.size() == 0)
    {
        cout << "EMPTY MAP" << endl;
        return;
    }
    cout << "================================";
    for (auto i : seeder_map)
    {
        cout << endl
             << i.first << endl;
        for (auto j : i.second)
            cout << j.first << endl
                 << j.second << endl;
    }
    cout << "================================" << endl;
}





void serve(int cl_soc)
{
    char buffer[1024] = {0};
    int n = read(cl_soc, buffer, 1024);
    if (n <= 0)
    {
        shutdown(cl_soc, SHUT_RDWR);
        close(cl_soc);
        return;
    }
    vector<string> client_req;
    char *token = strtok(buffer, SEP.c_str());
    while (token)
    {
        client_req.push_back(token);
        token = strtok(NULL, SEP.c_str());
    }
    string key_hash = client_req[1];
    string cl_socket = client_req[2];
    string file_path = client_req[3];
    string buf_buf = key_hash + SEP + cl_socket + SEP + file_path;
    writeLog("Request : " + buf_buf);
    if (client_req[0] == "0")
    {
        //share
        if (seeder_map.find(key_hash) == seeder_map.end())
        {
            seeder_map[key_hash][cl_socket] = file_path;
            append_to_seederlist(buf_buf);
        }
        else
        {
            if (seeder_map[key_hash].find(cl_socket) == seeder_map[key_hash].end())
            {
                seeder_map[key_hash][cl_socket] = file_path;
                append_to_seederlist(buf_buf);
            }
        }
    }
    else if (client_req[0] == "1")
    {
        //remove
        if (seeder_map.find(key_hash) != seeder_map.end())
        {
            map<string, string> temp;
            temp.insert(seeder_map[key_hash].begin(), seeder_map[key_hash].end());
            if (temp.size() == 1 && temp.find(cl_socket) != temp.end())
            {
                seeder_map.erase(key_hash);
                write_to_seederlist();
            }
            else if (temp.size() != 1)
            {
                seeder_map[key_hash].erase(cl_socket);
                write_to_seederlist();
            }
        }
    }
    else if (client_req[0] == "2")
    {
        //get
        string res;
        for (auto i : seeder_map[key_hash])
        {
            res = res + i.first + SEP + i.second + SEP;
        }
        writeLog("Response to GET :" + res);
        send(cl_soc, res.c_str(), res.size(), 0);
        writeLog("Data sent to client successfully :) ");
    }
    else if (client_req[0] == "3")
    {
        //EXIT
        map<string, map<string, string>>::iterator i;
        vector<map<string, map<string, string>>::iterator> del;
        for (i = seeder_map.begin(); i != seeder_map.end(); ++i)
        {
            if (i->second.find(cl_socket) != i->second.end())
            {
                i->second.erase(cl_socket);
            }
            if (i->second.size() == 0)
            {
                del.push_back(i);
            }
        }
        for (auto i : del)
        {
            seeder_map.erase(i);
        }
        write_to_seederlist();
    }
    writeLog("REQUEST " + client_req[0] + " SERVED");
    // print_map();
    shutdown(cl_soc, SHUT_RDWR);
    close(cl_soc);
    writeLog("Socket : " + to_string(cl_soc) + " closed successfully.");
    return;
}





vector<string>ArrayOfString(string s,char del)
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


void  create_user(int newsocketdes,string user_id,string pass)
{
	if(userReg.find(user_id)!=userReg.end())
	{
	  char status[]="0";
	  send(newsocketdes,status,sizeof(status),0);
	}
	else
	{
		char status[]="1";
		userReg[user_id]=pass;
	    send(newsocketdes,status,sizeof(status),0);

	}
	close(newsocketdes);
	goto l2;
	l2:
	  cout<<"";
}