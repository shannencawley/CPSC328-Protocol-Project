/************************************************************/
/* Author: Shannen Cawley, Kylee Hager, Dylan Derstine	 */
/* Creation Date: November 25, 2024			*/
/* Due Date: December 12, 2024				*/
/* Course: CPSC 328					*/
/* Professor Name: Dr. Schwesginer			*/
/* Assignment: 7					*/
/* Filename: clientmain.cpp				*/
/* Purpose: This file contains all the main function calls */
/* 	    necessary to connect with the server and run  */
/*          the capabilities of the file server application. */
/************************************************************/

#include "fileclient.h" 
#include <iostream>
#include <string>
#include <getopt.h>
#include <unistd.h>
using namespace std;

// tell them how to use the client
void printUsage() {
    std::cout << "Usage: ./fileclient -p <port> -h <host>\n";
}

int main(int argc, char *argv[]) {
    std::string port, host;
    int opt;

    // parsing command line args, should probably put in a function in fileclient.cpp
    // MAN PAGE: https://www.man7.org/linux/man-pages/man3/getopt.3.html
    while ((opt = getopt(argc, argv, "p:h:")) != -1) {
        switch (opt) {
            case 'p': // port
	      port = optarg;
	      break;
            case 'h': // host
	      host = optarg;
	      break;
            default:
	      printUsage(); // otherwise print usage. 
	      exit(EXIT_FAILURE);
        }
    }

    // if either arguement is empty
    // MAN PAGE: https://en.cppreference.com/w/cpp/iterator/empty
    if (port.empty() || host.empty()) {
        printUsage();
        return EXIT_FAILURE;
    }

    // create sockfd to hold info abt socket
    Client thisClient; // create instance of the Client class
    int sockfd = thisClient.connectToServer(host, port); // connect to server using port and host
    std::cout << "Connected to server at " << host << ":" << port << std::endl;

    
    std::string command; // hold user command
    while (true) {
      // this mocks the kernel
        std::cout << "> ";
	// MAN PAGE: https://en.cppreference.com/w/cpp/string/basic_string/getline
        std::getline(std::cin, command); // read user commands

        if (command == "exit") {
	    thisClient.sendCommand(sockfd, "exit", ""); // send exit command to server
            break;
        }
	// this is where all the types of commands and what they should do is at
        thisClient.handleCommand(command, sockfd); 
    }

    close(sockfd); // close socket
    return 0;
}
