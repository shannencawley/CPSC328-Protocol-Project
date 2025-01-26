/************************************************************/ 
/* Author: Shannen Cawley, Kylee Hager, Dylan Derstine */
/* Creation Date: November 25, 2024 */ 
/* Due Date: December 12, 2024 */ 
/* Course: CPSC 328 */ 
/* Professor Name: Dr. Schwesinger */ 
/* Assignment: 7 */
/* Filename: fileserver.cpp */
/* Purpose: This file contains all the server implementations*/
/* 	    necessary to accept client connections and handle*/ 
/*          file server operations. */
/************************************************************/ 

#include "fileserver.h"
#include <iostream>
#include <string>
#include <string_view> 
#include <vector>
#include <netdb.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <sstream>
#include <fstream>
#include <filesystem>
using namespace std;
using namespace std::filesystem;

bool startsWith(const std::string& str, const std::string& prefix) { if (str.size() < prefix.size()) return false; return str.compare(0, prefix.size(), prefix) == 0; }

// https://stackoverflow.com/questions/216823/how-to-trim-a-stdstring/34112965
string trimSpace(const string &str) {
   if (str.empty()) return str;
   // MAN PAGE: https://en.cppreference.com/w/cpp/types/size_t
   string::size_type i,j;
   i=0;
   while (i<str.size() && isspace(str[i])) ++i;
   if (i == str.size())
      return string(); // empty string
   j = str.size() - 1;
   //while (j>0 && isspace(str[j])) --j; // the j>0 check is not needed
   // MAN PAGE: https://en.cppreference.com/w/cpp/string/byte/isspace
   while (isspace(str[j])) --j;
   return str.substr(i, j-i+1);
}

bool Server::clientPathIsValid(path dir) {
	if(exists(dir)) {
		// MAN PAGE: https://en.cppreference.com/w/cpp/filesystem/canonical
		path tempDir = canonical(dir);
		if(startsWith(tempDir, serverDir)) {
			return true;
		}
	}
	return false;
}

Server::Server(string port, string directory) {
	cout << "Starting server" << endl;
	
	// set up the directory
	path directoryPath = path(directory);
	if(startsWith(directoryPath.string(), "./")) {
		if(directory == "./") {
			serverDir = current_path();
		} else {
			// MAN PAGE: https://cplusplus.com/reference/string/string/
			// MAN PAGE: https://en.cppreference.com/w/cpp/string/basic_string/substr
			// MAN PAGE: https://en.cppreference.com/w/cpp/filesystem/current_path
			serverDir = current_path() / directoryPath.string().substr(2);
		}
	// MAN PAGE: https://en.cppreference.com/w/cpp/filesystem/is_directory
	} else if(exists(directoryPath) && is_directory(directoryPath)) {
		serverDir = directoryPath;
	} else { // error :(
		perror("Given directory does not exist!");
		abort();
	}
	
	int server = createSocket(port);
	
	run(server);
	
	close(server);
}

Server::~Server() {
	cout << "Shutting down server" << endl;
}

void Server::run(int server) {
    // accept client connections
	// based off Beej's guide
    while(true) { // loop forever
        int client = accept(server, NULL, NULL);
        if (client == -1) {
            perror("accept");
            continue;
        }
		if (!fork()) { // this is the child process
			close(server); // child doesn't need the listener
			handleClient(client);
			close(client);
			exit(0);
		}
		close(client); // parent doesn't need this
    }
}

void Server::handleClient(int client) {
	path clientDir = serverDir;
	bool open = true;
	while(open) {
		Protocol request{};
		if(recvProtocol(client, request) != 1) {
			perror("Request failed");
		}
		cout << "THIS IS WHAT THE SERVER IS RECIEVING:\ncommand: " << request.command << endl << "length: " << request.length << endl << "data: " << request.data << endl;
		handleCommand(request, client, open, clientDir);
	}
}

void Server::handleCommand(Protocol protocol, int client, bool& open, path& clientDir) {
	// getting the command and data
	string command = trimSpace(protocol.command);
	string data = protocol.data;
	
	// match the command
	if(startsWith(command, "exit")) {
		cout << "exiting" << endl;
		open = false;
		sendResponse(client, "200", "Successful exit");
	} else if(startsWith(command, "cd")) {
		cd(client, clientDir, data);
	} else if(startsWith(command, "get -R")) {
		// only takes root paths
		string dir = removePrefix(command, "get -R ");
		
		getR(client, clientDir, dir);
		// signal that server done sending files
		sendResponse(client, "finished", "");
		// get the next request
		Protocol request{};
		if(recvProtocol(client, request) != 1) {
			perror("Request failed");
		}
	} else if(startsWith(command, "get")) {
		string dir = removePrefix(command, "get ");
		get(client, clientDir, dir);
	} else if(startsWith(command, "put")) {
		cout << "PUT" << endl;
		string tempCommand = removePrefix(command, "put ");
		put(client, tempCommand, data, clientDir);
	} else if(startsWith(command, "ls")) {
		ls(client, clientDir);
	} else if(startsWith(command, "pwd")) {
		pwd(client, clientDir);
	} else if(startsWith(command, "mkdir")) {
		mkdir(client, data, clientDir);
	} else { // catch all error 
		string response = "Command not found -\nCommand: " + protocol.command + "\nLength: " + to_string(protocol.length) + "\nData: " + protocol.data;
		sendResponse(client, "400", response);
	}
}

int Server::recvProtocol(int s, Protocol& protocol) {
	string command;
    char c;
    while (recv(s, &c, 1, 0) > 0) {
        if (c == '\n') {
            break;
        }
        command += c;
    }
	protocol.command = command;
	
	string lengthStr;
    while (recv(s, &c, 1, 0) > 0) {
        if (c == '\n') {
            break;
        }
        lengthStr += c;
    }
	
	int length;
	try {
        length = stoi(lengthStr);
		protocol.length = length;
    } catch (const std::invalid_argument& e) {
        std::cout << "Invalid argument for stoi: " << lengthStr << "\n" << protocol.length << "\n" << protocol.command << std::endl;
    } catch (const std::out_of_range& e) {
        std::cout << "Out of range: " << e.what() << std::endl;
    }
	
	int i = length;
	string data;
	while (i > 0) {
		recv(s, &c, 1, 0);
		data += c;
		i -= 1;
	}
	protocol.data = data;
	
	return 1;
}

void Server::sendResponse(int client, string status, string message) {
	string response = status + "\n" + to_string(message.length()) + "\n" + message;
	int send = sendall(client, response);
	if(send == -1) {
		perror("Failed to send");
	}
}

// cite Beej's guide
// MAN PAGE: https://beej.us/guide/bgnet/html//#sendrecv
// modified w.r.t len parameter
int Server::sendall(int s, string msg) {
	const char* buf = msg.c_str();
	int len = msg.size();
	int total = 0;        // how many bytes we've sent
	int bytesleft = len;  // how many we have left to send
	int n;

	while(total < len) {
		n = send(s, buf+total, bytesleft, 0);
		if (n == -1) { break; }
		total += n;
		bytesleft -= n;
	}

	// return -1 on failure, number of bytes sent on success
	return n == -1 ? -1 : total;
}

// author: Dr. Schwesinger
int Server::createSocket(string port) {
    struct addrinfo hints, *res;
    int fd;
    int retval;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    // MAN PAGE: https://www.man7.org/linux/man-pages/man3/getaddrinfo.3.html
    if ((retval = getaddrinfo(NULL, port.c_str(), &hints, &res)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(retval));
        exit(EXIT_FAILURE);
    }

    // MAN PAGE: https://man7.org/linux/man-pages/man2/socket.2.html
    if ((fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1) {
        perror("socket");
        freeaddrinfo(res);
        exit(EXIT_FAILURE);
    }

    // MAN PAGE: https://www.man7.org/linux/man-pages/man2/bind.2.html
    if (bind(fd, res->ai_addr, res->ai_addrlen) == -1) {
        perror("bind");
        freeaddrinfo(res);
        close(fd);
        exit(EXIT_FAILURE);
    }

    // MAN PAGE: https://www.man7.org/linux/man-pages/man2/listen.2.html
    if (listen(fd, 5) == -1) {
        perror("listen");
        freeaddrinfo(res);
        close(fd);
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(res);
    return fd;
}

void Server::put(int client, string command, string data, path clientDir) {
	if(startsWith(command, "-f")) { // is a file
		string dir = removePrefix(command, "-f ");
		string tempDir = serverDir.string() + dir;
		if(startsWith(dir, "./")) {tempDir = clientDir.string() + "/" + removePrefix(dir, "./");}
		if(!exists(tempDir)) {
			// source: https://www.geeksforgeeks.org/file-system-library-in-cpp-17/
			ofstream file(tempDir); 
			if (file.is_open()) { 
				// Write data to the file 
				file << data; 
				file.close(); 
				sendResponse(client, "200", "success");
			} 
			else { 
				// Handle the case if any error occured 
				cerr << "Failed to create file: " << tempDir 
					 << endl; 
				sendResponse(client, "400", "error");
			} 
		} else {sendResponse(client, "400", "File already exists");}
	} else if(startsWith(command, "-d")) { // is a directory
		string dir = removePrefix(command, "-d ");
		string tempDir = serverDir.string() + dir;
		if(startsWith(dir, "./")) {
			tempDir = clientDir.string() + removePrefix(dir, ".");
		}
		
		cout << "tempdir: " << tempDir << endl;
		if(!exists(tempDir)) {
			create_directory(tempDir);
			sendResponse(client, "200", "success");
		} else {
			sendResponse(client, "400", "Directory exists");
		}
	} else { // error, not a file or directory
		sendResponse(client, "400", "Could not read protocol");
	}
}
