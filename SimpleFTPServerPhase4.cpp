#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>

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

    fd_set readFds;
    int maxFd = serverSocket;
    FD_ZERO(&readFds);
    FD_SET(serverSocket, &readFds);

    while (true) {
        fd_set tempReadFds = readFds;
        if (select(maxFd + 1, &tempReadFds, NULL, NULL, NULL) == -1) {
            printError("Error in select");
            exit(2);
        }

        for (int i = 0; i <= maxFd; ++i) {
            if (FD_ISSET(i, &tempReadFds)) {
                if (i == serverSocket) {
                    // accept incoming connection
                    struct sockaddr_in clientAddress;
                    socklen_t clientAddressLength = sizeof(clientAddress);
                    int clientSocket = accept(serverSocket, (struct sockaddr*) &clientAddress, &clientAddressLength);
                    if (clientSocket < 0) {
                        printError("Error accepting incoming connection");
                        exit(2);
                    }
                    cout << "Client: " << inet_ntoa(clientAddress.sin_addr) << ":" << ntohs(clientAddress.sin_port) << endl;

                    FD_SET(clientSocket, &readFds);
                    if (clientSocket > maxFd) {
                        maxFd = clientSocket;
                    }
                } 
                else{
                    char readbuffer[1024];
                    memset(readbuffer,0,1024);
                    if (recv(i, readbuffer, 1024,0)<0){
                        printError("Error in receiving file get request");
                        exit(3);
                    }
                    stringstream Command(readbuffer);
                    string commandtype, file_name;
                    Command >> commandtype >> file_name;
                    if(commandtype=="get"){
                    cout << "FileRequested: " << file_name << endl;
                    // open requested file
                ifstream ifs(file_name, ios::binary);
                if (!ifs.good()) {
                    cerr << "Error opening file" << endl;
                    cerr << "FileTransferFail" << endl;
                    close(i);
                    FD_CLR(i, &readFds); // remove the socket from the master set
                    continue;
                }
                //getting file size
                ifs.seekg(0, ios::end);
                int file_size = ifs.tellg();
                ifs.seekg(0, ios::beg);
                          
                ostringstream oss;
                // oss << file_size;
                string file_size_string = oss.str();
                send(i, file_size_string.c_str(), file_size_string.size(), 0);
      
    
                // send file to client
                int total_bytes_sent = 0;
                while (!ifs.eof()) {
                    ifs.read(readbuffer,1024);
                    send(i, readbuffer, ifs.gcount(), 0);
                    total_bytes_sent += ifs.gcount();
                }
                cout << "TransferDone: " << total_bytes_sent << " bytes" << endl;

                ifs.close();

                close(i);
                FD_CLR(i, &readFds); // remove the socket from the master set
            }
        
            else if (commandtype == "put") {
                cout << "FileReceived: " << file_name << endl;

    // Receive file contents from client
    char writebuffer[1024];
    memset(writebuffer, 0, 1024);
    int total_bytes_received = 0;
    int bytes_received;
    ofstream ofs(file_name, ios::binary);
    
    while (true) {
        bytes_received = recv(i, writebuffer, 1024, 0);
        if (bytes_received < 0) {
            printError("Error in receiving file contents");
            exit(3);
        }
        total_bytes_received += bytes_received;
        ofs.write(writebuffer, bytes_received);
        if (bytes_received==0) break;
    }


    ofs.close();
    cout << "FileWriteDone: " << total_bytes_received << " bytes" << endl;

    close(i);
    FD_CLR(i, &readFds); // remove the socket from the master set
}

        }
    }}}
    close(serverSocket);
    }