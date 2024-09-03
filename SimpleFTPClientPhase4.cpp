#include <iostream>
#include <fstream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <chrono>
#include <thread>

using namespace std;

int main(int argc, char* argv[]) {

    if (argc != 5) {
        cerr << "Usage: " << argv[0] << " serverIPAddr:port operation{get/put} fileName receiveInterval\n";
        exit(1);
    }

    // parse server IP address, port number, operation, file name, and receive interval from command line arguments
    string serverIPAddr = strtok(argv[1], ":");
    int portNum = stoi(strtok(NULL, ":"));
    string operation = argv[2];
    string fileName = argv[3];
    int receiveInterval = stoi(argv[4]);

    // create TCP socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        cerr << "Error: Unable to create socket\n";
        exit(2);
    }

    // connect to server
    struct sockaddr_in serverAddress;
    memset(&serverAddress, '0', sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(portNum);
    inet_pton(AF_INET, serverIPAddr.c_str(), &serverAddress.sin_addr);

    if (connect(sockfd, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        cerr << "Error: Unable to connect to server\n";
        exit(2);
    }

    // print successful connection message
    cout << "ConnectDone: " << serverIPAddr << ":" << portNum << endl;

    // send operation and file name to server
    string request = operation + " " + fileName + "\0";
    send(sockfd, request.c_str(), request.length(), 0);

    if (operation == "get") {
  // send file name request to server
    // open file to write received data
    ofstream fileStream(fileName, ios::out | ios::binary);
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


    } else if (operation == "put") {
 
    // open file to read data to send
    ifstream fileStream(fileName, ios::in | ios::binary);
    if (!fileStream.is_open()) {
        cerr << "Error: Unable to read file\n";
        exit(3);
    }

    // send data from file to server with rate limiting
    int numBytesSent = 0;
    char buffer[1024];
    while (fileStream.good()) {
        fileStream.read(buffer,1024);
        int bytesRead = fileStream.gcount(); // get number of bytes read
        int bytesSent = send(sockfd, buffer, bytesRead, 0); // send data to server
        if (bytesSent == -1) {
            cerr << "Error: Failed to send data to server\n";
            exit(4);
        }
        numBytesSent += bytesSent;

        // Rate limiting: sleep for a short period of time
        // to control the sending rate
        usleep(1000*receiveInterval); // sleep for 1 ms

        // Check for any errors or interruption during file transfer
        if (!fileStream.good() && !fileStream.eof()) {
            cerr << "Error: Failed to read from file\n";
        }
    }

    // Close the file stream
    fileStream.close();

    cout << "File transfer complete. Sent " << numBytesSent << " bytes to server.\n";
}

}