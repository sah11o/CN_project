#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace std;


void printError(string msg, int code) {
    cerr << msg << endl;
    exit(code);
}

int main(int argc, char* argv[]) {
    if (argc != 3 || argv[2]==nullptr) {
        cerr << "Usage: ./SimpleFTPServerPhase1 portNum fileToTransfer" << endl;
        printError("Wrong number of command line arguments", 1);
    }

    int portNum = atoi(argv[1]);
    string fileToTransfer = argv[2];
    
    
//    int sockfd;
// struct sockaddr_in my_addr;
// sockfd = socket(PF_INET, SOCK_STREAM, 0);
// my_addr.sin_family = AF_INET; // host byte order
// my_addr.sin_port = htons(MYPORT); // short, network byte order
// my_addr.sin_addr.s_addr = inet_addr("10.0.0.1");
// memset(&(my_addr.sin_zero), '\0', 8); // zero the rest of the struct
// bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr));

    // create socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        printError("Error creating socket", 2);
    }

    // bind socket to port
    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(portNum);

    if (bind(sockfd, (struct sockaddr*) &serverAddress, sizeof(serverAddress)) < 0) {
        printError("Error binding to port", 2);
    }
    cout << "BindDone: " << portNum << endl;

    ifstream infile(fileToTransfer.c_str(), ios::binary);
    if (!infile.is_open()) {
        printError("File not present or not readable", 3);
    }
    // listen for incoming connections
    if (listen(sockfd, 1) < 0) {
        printError("Error listening for incoming connections", 2);
    }
    cout << "ListenDone: " << portNum << endl;

    // accept incoming connection
    struct sockaddr_in clientAddress;
    socklen_t clientAddressLength = sizeof(clientAddress);
    int clientSocket = accept(sockfd, (struct sockaddr*) &clientAddress, &clientAddressLength);
    if (clientSocket < 0) {
        printError("Error accepting incoming connection", 2);
    }
    cout << "Client: " << inet_ntoa(clientAddress.sin_addr) << ":" << ntohs(clientAddress.sin_port) << endl;

    // transfer file
    infile.seekg(0, infile.end);
    int fileSize = infile.tellg();
    infile.seekg(0, infile.beg);
    char buffer[fileSize];
    infile.read(buffer, fileSize);
    int bytesSent = send(clientSocket, buffer, fileSize, 0);
    if (bytesSent != fileSize) {
        printError("Error sending file", 2);
    }
    cout << "TransferDone: " << fileSize << " bytes" << endl;

    // close sockets and file
    close(clientSocket);
    close(sockfd);
    infile.close();

    return 0;
}
