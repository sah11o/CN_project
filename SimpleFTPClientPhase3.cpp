#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <chrono>
#include <thread>

using namespace std;

int main(int argc, char* argv[]) {

    if (argc != 4) {
        cerr << "Usage: " << argv[0] << " serverIPAddr:port fileToWrite receiveInterval\n";
        exit(1);
    }

    // parse server IP address, port number, and receive interval from command line arguments
    string serverIPAddr = strtok(argv[1], ":");
    int portNum = stoi(strtok(NULL, ":"));
    string fileToWrite = argv[2];
    int receiveInterval = stoi(argv[3]);

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

    // print successful connection message
    cout << "ConnectDone: " << serverIPAddr << ":" << portNum << endl;

    // send file name request to server
    string fileNameRequest = "get " + fileToWrite + "\0";
    send(sockfd, fileNameRequest.c_str(), fileNameRequest.length(), 0);

    // open file to write received data
    ofstream fileStream(fileToWrite, ios::out | ios::binary);
    if (!fileStream.is_open()) {
        cerr << "Error: Unable to create/write file\n";
        exit(3);
    }

    // receive data and write to file with rate limiting
    int numBytesReceived = 0;
    char buffer[1024];
    while (1) {
        int bytesRead = recv(sockfd, buffer, sizeof(buffer), 0);
        if (bytesRead < 0) {
            cerr << "Error: Unable to receive data from server\n";
            exit(3);
        }
        if (bytesRead == 0) {
            break;
        }
        fileStream.write(buffer, bytesRead);
        numBytesReceived += bytesRead;

        // Rate limiting using sleep
        usleep(receiveInterval* 1000);
         }

    // print successful file reception message
    cout << "FileWritten: " << numBytesReceived << " bytes" << endl;

    // close file and socket
    fileStream.close();
    close(sockfd);

    return 0;
}
