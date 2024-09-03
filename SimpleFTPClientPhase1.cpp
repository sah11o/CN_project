#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace std;

int main(int argc, char* argv[]) {

    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " [serverIPAddr:port] [fileToReceive]\n";
        exit(1);
    }

    // parse server IP address and port number from command line arguments
    string serverIPAddr = strtok(argv[1], ":");
    int portNum = stoi(strtok(NULL, ":"));
    string fileToReceive = argv[2];

    // create TCP socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        cerr << "Error: Unable to create socket\n";
        exit(2);
    }

    // connect to server
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(portNum);
    inet_pton(AF_INET, serverIPAddr.c_str(), &serverAddress.sin_addr);

    if (connect(sockfd, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        cerr << "Error: Unable to connect to server\n";
        exit(2);
    }

    // printing successful connection message
    cout << "ConnectDone: " << serverIPAddr << ":" << portNum << endl;

    // openening file to write received data
    ofstream fileStream(fileToReceive, ios::out | ios::binary);
    if (!fileStream.is_open()) {
        cerr << "Error: Unable to create/write file\n";
        exit(3);
    }

    // receive data and write to file
    int numBytesReceived = 0;
    char buffer[1024];
    while (1) {
        int bytesRead = recv(sockfd, buffer, sizeof(buffer), 0);
        if (bytesRead < 0) {
            cerr << "Error: Unable to receive file data from server\n";
            exit(3);
        }
        if (bytesRead == 0) {
            break;
        }
        fileStream.write(buffer, bytesRead);
        numBytesReceived += bytesRead;
    }

    // print successful file reception message
    cout << "FileWritten: " << numBytesReceived << " bytes" << endl;

    // close file and socket
    fileStream.close();
    close(sockfd);
    
    return 0;
}
