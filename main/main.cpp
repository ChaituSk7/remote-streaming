#include "header.h"
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#define PORT 8083

void sendExtenstionToClient (std::string extenstion) {

    int sockfd, new_socket;
    struct sockaddr_in servaddr, cliaddr;
    char *buf = NULL;
    buf = &extenstion[0];

    // Creating socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    int optval = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) {
        perror("");
        exit(EXIT_FAILURE);
    }

    // Assign IP, PORT
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("10.1.137.49");
    servaddr.sin_port = htons(PORT);

    // Bind the socket with the server address
    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    if (listen(sockfd, 5) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    // Accept the connection
    socklen_t len = sizeof(cliaddr);
    g_print("Waiting for clinet to connect....\n");
    if ((new_socket = accept(sockfd, (struct sockaddr *)&cliaddr, &len)) < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }  
    
    // Send message to client
    send(new_socket, buf, sizeof(buf), 0);
    
    // Close the socket
    close(sockfd);
}

int main(int argc, char *argv[]) {
    
    for(int i = 1; i < argc; i++) {
        char *path = argv[i];
        path = realpath(path, NULL);
        if (path != NULL) {
            std::string filename = argv[i];
            size_t pos = filename.find_last_of(".");
            if (pos != std::string::npos) {
                std::string extension = filename.substr(pos + 1);
                std::cout << "File extension is: " << extension << std::endl;
                sendExtenstionToClient(extension);  
                if (extension == "mp4") {
                    localhost_pipeline(path);
                } 
                else if (extension == "avi") {
                    hostavi_pipeline(path);
                }
                else if (extension == "mp3") {
                    hostmp3_pipeline(path);
                }
                else if (extension == "webm") {
                    hostwebm_pipeline(path);
                }
                else {
                    g_printerr("Unsuported format.\n");
                }
            }
        }
    }
    return 0;
}