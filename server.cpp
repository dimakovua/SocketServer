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
namespace fs = std::filesystem;
std::string Answering(char (&buf)[4096], int bytesRecv){
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
                if(i.find(k) == std::string::npos){
                    checker = 0;
                }
            }
            if(checker == 1){
                finally.push_back(i);
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
    int listening = socket(AF_INET, SOCK_STREAM, 0);
    if (listening < 0){
        std::cerr << "Listening error!";
        return -1;
    }
    sockaddr_in hint;
    hint.sin_port = htons(1031);
    hint.sin_family = AF_INET;
    inet_pton(AF_INET, "0.0.0.0", &hint.sin_addr);
    
    if(bind(listening, (sockaddr*)&hint, sizeof(hint)) == -1){
        std::cerr << "Bind Error!";
        return -2;
    }

    if(listen(listening, SOMAXCONN) < 0){
        std::cerr << "Listen Error!";
        return -3;
    }

    sockaddr_in client;
    socklen_t client_size = sizeof(client);
    char host[NI_MAXHOST];
    char svc[NI_MAXSERV];

    int clientSocket = accept(listening, (sockaddr*)&client, &client_size);
    if(clientSocket < 0){
        std::cerr << "Accepting error!";
        return -4;
    }
    close(listening);

    memset(host, 0, NI_MAXHOST);
    memset(svc, 0, NI_MAXSERV);

    int result = getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, svc, NI_MAXSERV, 0);
    if(result){
        std::cout << host << " connected on" << svc<<"\n";
    }
    else{
        inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
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
            break;
        }
        if(bytesRecv == 0){
            std::cout << "The client disconnected!\n";
            break;
        }
        else{
            std::cout << "Recieved: " << std::string(buf, 0, bytesRecv) << "\n";
            //Final part
            std::string stringAnswer = Answering(buf, bytesRecv);
            int answerSize = stringAnswer.length();
            char answer[answerSize+1];
            strcpy(answer, stringAnswer.c_str());
            send(clientSocket, answer, answerSize, 0);
        }
    }
    return 0;
}