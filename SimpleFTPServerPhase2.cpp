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

void printError(string msg) {
    cerr << msg << endl;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "Usage: ./SimpleFTPServerPhase2 [portNum]" << endl;
        printError("Wrong number of command line arguments");
        exit(1);
    }

    int portNum = atoi(argv[1]);

    // create socket
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        printError("Error creating socket");
        exit(2);
    }

    // bind socket to port
    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(portNum);

    if (bind(serverSocket, (struct sockaddr*) &serverAddress, sizeof(serverAddress)) < 0) {
        printError("Error binding to port");
        exit(2);
    }
    cout << "BindDone: " << portNum << endl;

    // listen for incoming connections
    if (listen(serverSocket, 5) < 0) {
        printError("Error listening for incoming connections");
        exit(2);
    }
    cout << "ListenDone: " << portNum << endl;

    while (true) {
        // accept incoming connection
        struct sockaddr_in clientAddress;
        socklen_t clientAddressLength = sizeof(clientAddress);
        int clientSocket = accept(serverSocket, (struct sockaddr*) &clientAddress, &clientAddressLength);
        if (clientSocket < 0) {
            printError("Error accepting incoming connection");
            exit(2);
        }
        cout << "Client: " << inet_ntoa(clientAddress.sin_addr) << ":" << ntohs(clientAddress.sin_port) << endl;

        // receive command from client
        char commandBuffer[1024];
        int bytesReceived = recv(clientSocket, commandBuffer, 1024, 0);
        if (bytesReceived < 0) {
            printError("Error receiving command from client");
            close(clientSocket);
            continue;
        }
        commandBuffer[bytesReceived] = '\0';
        string command = commandBuffer;
        if (command.substr(0, 4) != "get ") {
            cout << "UnknownCmd" << endl;
            printError("Unknown command from client");
            close(clientSocket);
            continue;
        }
        string fileName = command.substr(4);
        cout << "FileRequested: " << fileName << endl;

        // check if file is present and readable
        ifstream infile(fileName.c_str(), ios::binary);
        if (!infile.is_open()) {
            cout << "FileTransferFail" << endl;
            printError("File not present or not readable");
            close(clientSocket);
            continue;
        }

        // transfer file
        infile.seekg(0, infile.end);
        int fileSize = infile.tellg();
        infile.seekg(0, infile.beg);
        char buffer[fileSize];
        infile.read(buffer, fileSize);
        int bytesSent = send(clientSocket, buffer, fileSize, 0);
        if (bytesSent != fileSize) {
              printError("Error sending file not all bytes transfered");
        }
        cout << "TransferDone: " << fileSize << " bytes" << endl;

    // close sockets and file
        close(clientSocket);
        infile.close();
}
}
