#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>
#include <filesystem>
#include <vector>
#include <fstream>
namespace fs = std::filesystem;
std::string Answering(char (&buf)[4096], int bytesRecv, std::ofstream& out){
    std::string tmp = "Unknown command!";
    if(buf[0] == 'g' && buf[1] == '|'){
        tmp = "";
        std::string file_path = fs::current_path();
    
        for (const auto & entry : fs::directory_iterator(file_path)){
            tmp+=entry.path().filename();
            tmp+=' ';
        }
        //std::cout << tmp;
    }
    else if(buf[0] == 'W' && buf[1] == '|'){
        tmp = "Created by Dmytro Kovalenko K-27. Variant #6. List of files. Type \"#\" to disconnect. Type \"Get\" to get a list of files in current directory. Type \"Get smth\" to get list of files which names contain \"smth\"";
    }
    if(buf[0] == 'G' && buf[1] == '|'){
        tmp = "";
        std::vector<std::string> list_of_files;
        std::string file_path = fs::current_path();
        std::vector<std::string> list_of_filters;
        std::string word_filter = "";
        for(int i = 2; i < bytesRecv; i++){
            if(buf[i] == '|'){
                list_of_filters.push_back(word_filter);
                word_filter = "";
            }
            else{
                word_filter += buf[i];
            }
        }
        for (const auto & entry : fs::directory_iterator(file_path)){
            list_of_files.push_back(entry.path().filename());
        }
        std::vector<std::string> finally;
        bool checker = 1;
        for(auto i : list_of_files){
            checker = 1;
            for(auto k : list_of_filters){
                out << "Searching " << k << " in " << i << std::endl;
                if(i.find(k) == std::string::npos){
                    out << "There isn't " << k << " in " << i << std::endl;
                    checker = 0;
                }
                else{
                    out << "There is " << k << " in " << i << std::endl;
                }
            }
            if(checker == 1){
                out << i << " is OK!" << std::endl;
                finally.push_back(i);
            }
            else{
                out << i << " is NOT OK!((((" << std::endl;
            }
        }
        if(finally.size() != 0){
            for(auto i : finally){
                tmp += i;
                tmp += '\t';
            }
        }
        else{
            tmp = "Files are not found!";
        }
    }
    return tmp;
}

int main(){
    std::ofstream out;
    
    out.open("/Users/dmitrikovalenko/SocketServer/LoggerServer.txt");
    if (out.is_open())
    {
        out << "Hello World!" << std::endl;
    }
    int listening = socket(AF_INET, SOCK_STREAM, 0);
    if (listening < 0){
        out << "SERVER> Listening error!" << std::endl;
        std::cerr << "Listening error!";
        return -1;
    }
    sockaddr_in hint;
    hint.sin_port = htons(1031);
    hint.sin_family = AF_INET;
    inet_pton(AF_INET, "0.0.0.0", &hint.sin_addr);
    
    if(bind(listening, (sockaddr*)&hint, sizeof(hint)) == -1){
        out << "SERVER> Bind Error!" << std::endl;
        std::cerr << "Bind Error!";
        return -2;
    }

    if(listen(listening, SOMAXCONN) < 0){
        out << "SERVER> Listen Error!" << std::endl;
        std::cerr << "Listen Error!";
        return -3;
    }

    sockaddr_in client;
    socklen_t client_size = sizeof(client);
    char host[NI_MAXHOST];
    char svc[NI_MAXSERV];

    int clientSocket = accept(listening, (sockaddr*)&client, &client_size);
    if(clientSocket < 0){
        out << "SERVER> Accepting Error!" << std::endl;
        std::cerr << "Accepting error!";
        return -4;
    }
    close(listening);

    memset(host, 0, NI_MAXHOST);
    memset(svc, 0, NI_MAXSERV);

    int result = getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, svc, NI_MAXSERV, 0);
    if(result){
        out <<"SERVER> " << host << " connected on " << svc<< std::endl;
        std::cout << host << " connected on " << svc<<"\n";
    }
    else{
        inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
        out << "SERVER> " << host << " connected on " << ntohs(client.sin_port) << std::endl;
        std::cout << host << " connected on" << ntohs(client.sin_port) << "\n";
    }
    char buf[4096];
    char answer[4096];
    while(true){
        memset(buf, 0, 4096);
        memset(answer, 0, 4096);
        int bytesRecv = recv(clientSocket, buf, 4096, 0);
        if(bytesRecv < 0 ){
            std::cerr << "There is a connection issue!\n"; 
            out << "SERVER> There is a connection issue!"<< std::endl; 
            break;
        }
        if(bytesRecv == 0){
            std::cout << "The client disconnected!\n";
            out << "SERVER> The client disconnected!"<< std::endl; 
            break;
        }
        else{
            std::cout << "Recieved: " << std::string(buf, 0, bytesRecv) << "\n";
            out << "SERVER> Recieved: " << std::string(buf, 0, bytesRecv) << std::endl;
            //Final part
            std::string stringAnswer = Answering(buf, bytesRecv, out);
            int answerSize = stringAnswer.length();
            char answer[answerSize+1];
            //out << "SERVER> Answering: " << stringAnswer << std::endl;
            strcpy(answer, stringAnswer.c_str());
            send(clientSocket, answer, answerSize, 0);
            out << "SERVER> Answer has been sent!" << std::endl;
        }
    }
    return 0;
}