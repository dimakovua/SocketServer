#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>
#include <vector>

using namespace std;

bool toStop = false;
vector<string> Spliting(string userInput){
    string tmp = "";
    vector<string> Line;
    for(int i = 0; i < userInput.length(); i++){
        if(userInput[i] == ' '){
            Line.push_back(tmp);
            tmp = "";
        }
        else if(i == userInput.length()-1){
            tmp+=userInput[i];
            Line.push_back(tmp);
            tmp = "";
        }
        else{
            tmp+= userInput[i];
        }
    }
    return Line;
}
string Analysis(string userInput){
    string result;
    vector<string> Line = Spliting(userInput);
    if(Line[0] == "#"){
        toStop = true;
        result = "Memememe";
        return result;
    }
    else if(Line[0] == "Who" && Line.size() == 1)
    {
        result = "W|";
        return result;
    }
    else if(Line[0] == "Get")
    {
        if(Line.size() == 1)
        {
            result = "g|";
            return result;
        }
        else
        {
            result = "G|";
            for(int k = 1; k < Line.size(); k++){
                //std::cout << "Line size is: " << Line.size() << std::endl;
                result+= Line[k];
                result+= "|";
                
            }
            return result;
        }
    }
    else{
        result = "?";
        return result;
    }
}
int main()
{
    //	Create a socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        return 1;
    }

    //	Create a hint structure for the server we're connecting with
    int port = 1031;
    string ipAddress = "127.0.0.1";

    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(port);
    inet_pton(AF_INET, ipAddress.c_str(), &hint.sin_addr);

    //	Connect to the server on the socket
    int connectRes = connect(sock, (sockaddr*)&hint, sizeof(hint));
    if (connectRes == -1)
    {
        return 1;
    }

    //	While loop:
    char buf[4096];
    string userInput;


    do {
        //		Enter lines of text
        cout << "> ";
        getline(cin, userInput);
        userInput = Analysis(userInput);
        if(toStop == true){
            std::cout << "You chose to stop!";
            continue;
        }
        //		Send to server
        int sendRes = send(sock, userInput.c_str(), userInput.size() + 1, 0);
        if (sendRes == -1)
        {
            cout << "Could not send to server! Whoops!\r\n";
            continue;
        }

        //		Wait for response
        memset(buf, 0, 4096);
        int bytesReceived = recv(sock, buf, 4096, 0);
        if (bytesReceived == -1)
        {
            cout << "There was an error getting response from server\r\n";
        }
        else
        {
            //		Display response
            cout << "SERVER> " << string(buf, bytesReceived) << "\r\n";
        }
    } while(!toStop);

    //	Close the socket
    close(sock);

    return 0;
}