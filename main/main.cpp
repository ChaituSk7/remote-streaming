#include "header.h"
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#define PORT 8083

// Send extention of a stream to client using socket
void sendExtenstionToClient(std::string extenstion) {

	int sockfd, new_socket;
	struct sockaddr_in servaddr, cliaddr;
	char *buf = NULL;
	buf = &extenstion[0];

	// Creating socket file descriptor
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
		perror("socket failed");
		exit (EXIT_FAILURE);
	}

	int optval = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval))
			== -1) {
		perror("");
		exit (EXIT_FAILURE);
	}

	// Assign IP, PORT
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr("10.1.139.244");
	servaddr.sin_port = htons(PORT);

	// Bind the socket with the server address
	if (bind(sockfd, (const struct sockaddr*) &servaddr, sizeof(servaddr))
			< 0) {
		perror("bind failed");
		exit (EXIT_FAILURE);
	}

	// Listen for connections
	if (listen(sockfd, 5) < 0) {
		perror("listen");
		exit (EXIT_FAILURE);
	}

	// Accept the connection
	socklen_t len = sizeof(cliaddr);
	g_print("Waiting for clinet to connect....\n");
	if ((new_socket = accept(sockfd, (struct sockaddr*) &cliaddr, &len)) < 0) {
		perror("accept");
		exit (EXIT_FAILURE);
	}

	// Send message to client
	send(new_socket, buf, sizeof(buf), 0);

	// Close the socket
	close(sockfd);
}

// Main function to play files from given directory path through command line
int main(int argc, char *argv[]) {
	DIR *dir;
	struct dirent *ent;
	char *uri = argv[1];
	uri = realpath(uri, NULL);
	cout << "uri: " << uri << endl;
	// opens directory
	if ((dir = opendir(uri)) != NULL) {
		// Read file from directory
		while ((ent = readdir(dir)) != NULL) {
			if (ent->d_type != DT_REG) {
				continue;
			}
			string filename = ent->d_name;
			char *val = ent->d_name;
			cout << "val: " << val << endl;
			size_t pos = filename.find_last_of(".");
			if (pos != std::string::npos) {
				std::string extension = filename.substr(pos + 1);
				std::cout << "File extension is: " << extension << std::endl;
				string path = string(uri) + "/" + string(val);

				// Function to send extension to client
				sendExtenstionToClient(extension);

				// Call host pipeline by checking extension
				if (extension == "mp4") {
					// Capture image from stream and store
					directory_set(path.c_str());
					// Display capture image on host side
					//display_img();
					// Send image to client side
					image_display();
					// Play mp4 file
					localhost_pipeline(path.c_str());
				} else if (extension == "avi") {
					// Play avi file
					directory_set(path.c_str());
					//display_img();
					image_display();
					hostavi_pipeline(path.c_str());
				} else if (extension == "mp3") {
					// Play mp3 file
					hostmp3_pipeline(path.c_str());
				} else if (extension == "webm") {
					// Play webm file
					directory_set(path.c_str());
					//display_img();
					image_display();
					hostwebm_pipeline(path.c_str());
				} else {
					g_printerr("Unsuported format.\n");
				}
			}
		}
	}
	return 0;
}
