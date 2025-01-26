/************************************************************/
/* Author: Shannen Cawley, Kylee Hager, Dylan Derstine      */
/* Creation Date: November 25, 2024                        */
/* Due Date: December 12, 2024                             */
/* Course: CPSC 328                                        */
/* Professor Name: Dr. Schwesinger                         */
/* Assignment: 7                                           */
/* Filename: fileclient.cpp                                */
/* Purpose: This file contains all the client implementations */
/*          necessary to connect with the server and run   */
/*          the capabilities of the file server application. */
/************************************************************/

#include "fileclient.h"
#include <iostream>
#include <vector>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <dirent.h> // directory traversal, MAN PAGE: https://man7.org/linux/man-pages/man0/dirent.h.0p.html
#include <sys/stat.h> // for mkdir
#include <filesystem> // MAN PAGE: https://en.cppreference.com/w/cpp/filesystem
#include <fstream>
#include <string>
using namespace std;
using namespace std::filesystem; // MAN PAGE: https://en.cppreference.com/w/cpp/filesystem

// src: https://medium.com/@stefano.fiore84/a-generic-implementation-of-ends-with-for-modern-c-containers-bd7f7eb5cda8
bool endsWith(std::string const &value, std::string const &ending)
{
    // MAN PAGE: https://en.cppreference.com/w/cpp/iterator/size
    if (ending.size() > value.size())
    { return false; }
	// MAN PAGE: https://en.cppreference.com/w/cpp/algorithm/equal
	// bool equal( InputIt1 first1, InputIt1 last1, InputIt2 first2 );
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

// MAN PAGE: https://en.cppreference.com/w/cpp/string/basic_string/npos
// MAN PAGE: https://en.cppreference.com/w/cpp/string/basic_string/substr
std::string getFileName(const std::string& path) { size_t pos = path.find_last_of('/'); if (pos != std::string::npos) { return path.substr(pos + 1); } return path; } // If no '/' is found, return the original string

string readFile(const string& dir) {
    // MAN PAGE: https://en.cppreference.com/w/cpp/io/basic_ifstream
    ifstream f(dir);

    // check if the file is successfully opened
    // MAN PAGE: https://en.cppreference.com/w/cpp/io/basic_fstream/is_open
    if (!f.is_open()) {
        cerr << "Error opening the file!";
        return "";
    }

    // string variable to store the read data
    stringstream file;
    string s;

    // read each line of the file and append it to the string stream
    // MAN PAGE: https://en.cppreference.com/w/cpp/string/basic_string/getline
    while (getline(f, s))
        file << s << "\n";

    // close the file
    // MAN PAGE: https://www.man7.org/linux/man-pages/man2/close.2.html
    f.close();
 
    return file.str();
}
// 
std::string Client::trimSpace(const std::string &str) {
    // MAN PAGE: https://en.cppreference.com/w/cpp/iterator/empty
    if (str.empty()) return str;
    
    // MAN PAGE: https://en.cppreference.com/w/cpp/types/size_t
    // std::size_t is the unsigned integer type of the result of the sizeof operator as well as the sizeof... 
    // operator and the alignof operator(since C++11).
    std::string::size_type i = 0, j = str.size() - 1;

    // remove leading whitespace
    // MAN PAGE: https://en.cppreference.com/w/cpp/string/byte/isspace
    while (i < str.size() && std::isspace(str[i])) ++i;

    // if the string is entirely whitespace
    if (i == str.size()) return std::string();

    // remove trailing whitespace
    // MAN PAGE: https://en.cppreference.com/w/cpp/string/byte/isspace
    while (std::isspace(str[j])) --j;

    // MAN PAGE: https://en.cppreference.com/w/cpp/string/basic_string/substr
    return str.substr(i, j - i + 1);
}

bool startsWith(const std::string& str, const std::string& prefix) { if (str.size() < prefix.size()) return false; return str.compare(0, prefix.size(), prefix) == 0; }

string removePrefix(string str1, string prefix) { str1.erase(0, prefix.length()); return str1; }

string toPath(string dir) {
	if(startsWith(dir, "./")) {
		// MAN PAGE: https://en.cppreference.com/w/cpp/filesystem/current_path
		string fullDir = path(current_path() / removePrefix(dir, "./")).string();
		return fullDir;
	}
	return dir;
}

void Client::putR(int sockfd, string ogLocalPath, string localPath, string remotePath) {
	// localPath is full path, no './'
	// MAN PAGE: https://en.cppreference.com/w/cpp/filesystem/is_directory
	// MAN PAGE: https://en.cppreference.com/w/cpp/filesystem/exists
	if(exists(localPath) && is_directory(localPath)) {
		// send top lvl directory
		put(sockfd, localPath, remotePath);
		// MAN PAGE: https://en.cppreference.com/w/cpp/filesystem/directory_iterator
		for(const auto& entry : directory_iterator(localPath)) {
			// remote path should be ./localPath - ogLocalPath
			// MAN PAGE: https://en.cppreference.com/w/cpp/filesystem/path
			put(sockfd, entry.path(), remotePath + removePrefix(entry.path(), ogLocalPath));
			// MAN PAGE: https://en.cppreference.com/w/cpp/filesystem/is_directory
			// MAN PAGE: https://en.cppreference.com/w/cpp/filesystem/path
			if(is_directory(entry.path())) {
				putR(sockfd, ogLocalPath, entry.path(), remotePath);
			}
		}
	}
}

void Client::put(int sockfd, string localPath, string remotePath) {
	// MAN PAGE: https://en.cppreference.com/w/cpp/filesystem/is_directory
	// MAN PAGE: https://en.cppreference.com/w/cpp/filesystem/path
	if(is_directory(path(localPath))) {
		// isolate file name
		string fileName;
		// MAN PAGE: https://en.cppreference.com/w/cpp/types/size_t
		// MAN PAGE: https://en.cppreference.com/w/cpp/string/basic_string/find_last_of
		size_t pos = localPath.find_last_of("/");
		// MAN PAGE: https://en.cppreference.com/w/cpp/string/basic_string/npos
		if (pos != std::string::npos) { fileName = localPath.substr(pos + 1); }
		
		// if remote path ends with '/', then the file is concatnatnanttned to it
		// MAN PAGE: https://en.cppreference.com/w/cpp/container/vector/back
		if(remotePath.back() == '/') { remotePath += fileName; }
		
		if (!sendCommand(sockfd, "put -d " + remotePath, "")) {
			std::cerr << "Error sending put command to server." << std::endl;
			return;
		}
	} else {
		// read the local file
		string fileContents = readFile(localPath);
		
		// isolate file name
		string fileName;
		// MAN PAGE: https://en.cppreference.com/w/cpp/string/basic_string/find_last_of
		size_t pos = localPath.find_last_of("/");
		// MAN PAGE: https://en.cppreference.com/w/cpp/string/basic_string/npos
		if (pos != std::string::npos) { fileName = localPath.substr(pos + 1); }
		
		// if remote path ends with '/', then the file is concatnatnanttned to it
		if(remotePath.back() == '/') { remotePath += fileName; }
	
		// Send the file data to the server
		if (!sendCommand(sockfd, "put -f " + remotePath, fileContents)) {
			std::cerr << "Error sending put command to server." << std::endl;
			return;
		}
	}

	Protocol recPac;
	// MAN PAGE; https://en.cppreference.com/w/cpp/language/this
	recvProtocol(this->socketFD, recPac);
	// MAN PAGE: https://en.cppreference.com/w/cpp/container/vector/data
	std::cout << recPac.data << std::endl;
}

void Client::getR(int sockfd, string ogLocalPath, string localPath, string remotePath) {
	// localPath is full path, no './'
	cout << "local path: " << localPath << endl;
	cout << "remote path: " << remotePath << endl;
	// MAN PAGE: https://en.cppreference.com/w/cpp/filesystem/exists
	if(!exists(localPath)) {
		// send top lvl directory
		get(sockfd, localPath, remotePath);
		for(const auto& entry : directory_iterator(localPath)) {
			// remote path should be ./localPath - ogLocalPath
			get(sockfd, entry.path(), remotePath + removePrefix(entry.path(), ogLocalPath));
			if(is_directory(entry.path())) {
				getR(sockfd, ogLocalPath, entry.path(), remotePath);
			}
		}
	}
}

void Client::get(int sockfd, string localPath, string remotePath) {
	// convert to full path if it's ./
	localPath = toPath(localPath);

	// request file from server
	if (!sendCommand(sockfd, "get " + remotePath, "")) {
		std::cerr << "Error sending get command to server." << std::endl;
		return;
	}

	Protocol recPac;
	recvProtocol(this->socketFD, recPac);
	
	if(recPac.command != "400") {
		string response = recPac.command;
		istringstream stream(response);
		string type, path;
		stream >> type;
		stream >> path;
		
		string dir = localPath;
		
		if(!exists(dir)) {
			if(type == "-f") {
				// source: https://www.geeksforgeeks.org/file-system-library-in-cpp-17/
				ofstream file(dir); 
				if (file.is_open()) { 
					// write data to the file 
					file << recPac.data; 
					// MAN PAGE; https://www.man7.org/linux/man-pages/man2/close.2.html
					file.close(); 
					
					cout << "Retrieved file" << endl;
				} 
				else { 
					// handle the case if any error occured 
					cerr << "Failed to create file: " << dir 
						 << endl; 
				}
			} else if(type == "-d") {
				// MAN PAGE: https://en.cppreference.com/w/cpp/filesystem/create_directory
				create_directory(localPath);
				cout << "creating directory: " << localPath << endl;
			} else { cout << "something went wrong " << response << endl; }
		} else {
			cout << "dir alr exists" << endl;
		}

	} else {
		cout << "error " << recPac.data << endl;
	}
}


// really send function
// from Dr.Schwesinger's project 5 solution 
int Client::reallySend(int sockfd, const char* buf, int len) {
    int total = 0;
    int bytesLeft = len;
    int n;

    while (total < len) {
        n = send(sockfd, buf + total, bytesLeft, 0);
        if (n == -1) break;
        total += n;
        bytesLeft -= n;
    }

    return n == -1 ? -1 : total;
}

// really receive function
// from Dr.Schwesinger's project 5 solution
int Client::reallyRecv(int sockfd, char* buf, int len) {
    int total = 0;
    int bytesLeft = len;
    int n;

    while (total < len) {
        n = recv(sockfd, buf + total, bytesLeft, 0);
        if (n <= 0) break;
        total += n;
        bytesLeft -= n;
    }

    return n == -1 ? -1 : total;
}

// create socket, connect to the server
int Client::connectToServer(const std::string &host, const std::string &port) {
    struct addrinfo hints, *res; // struct to hold socket info
    int sockfd;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; // IPV4
    hints.ai_socktype = SOCK_STREAM; // TCP

    // resolve host to IPv4 address
    // MAN PAGE; https://www.man7.org/linux/man-pages/man3/getaddrinfo.3.html
    if (getaddrinfo(host.c_str(), port.c_str(), &hints, &res) != 0) {
	// MAN PAGE: https://www.man7.org/linux/man-pages/man3/strerror.3.html
        std::cerr << "getaddrinfo failed: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }

    // MAN PAGE: https://man7.org/linux/man-pages/man2/socket.2.html
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd == -1) {
        std::cerr << "Socket creation failed: " << strerror(errno) << std::endl;
	// MAN PAGE: https://www.man7.org/linux/man-pages/man3/freeaddrinfo.3p.html
        freeaddrinfo(res);
        exit(EXIT_FAILURE);
    }

    // MAN PAGE: https://www.man7.org/linux/man-pages/man2/connect.2.html
    if (connect(sockfd, res->ai_addr, res->ai_addrlen) == -1) {
        std::cerr << "Connection failed: " << strerror(errno) << std::endl;
        close(sockfd);
	// MAN PAGE: https://www.man7.org/linux/man-pages/man3/freeaddrinfo.3p.html
        freeaddrinfo(res);
	// MAN PAGE: https://en.cppreference.com/w/cpp/utility/program/exit
        exit(EXIT_FAILURE);
    }
	
	// set up the directory
	// MAN PAGE: https://en.cppreference.com/w/cpp/filesystem
	clientDir = filesystem::current_path();

	this->socketFD = sockfd;
    freeaddrinfo(res);
    return sockfd;
}

// send a command to the server
bool Client::sendCommand(int sockfd, const std::string &command, const std::string &data) {
    std::string request =
      //"Command: "
      command + "\n" 
      //"Length: "
      // MAN PAGE: https://en.cppreference.com/w/cpp/string/basic_string/to_string
      + std::to_string(data.size()) + "\n"
      // "Data: "
      + data;

    return reallySend(sockfd, request.c_str(), request.size()) == (int)request.size();
}

// receive a response from the server
std::string Client::receiveResponse(int sockfd) {
    char buffer[4096]; // buffer this time 
    // MAN PAGE: https://en.cppreference.com/w/cpp/language/sizeof
    int received = reallyRecv(sockfd, buffer, sizeof(buffer));
    if (received <= 0) {
        std::cerr << "Failed to receive response." << std::endl;
        return "";
    }
    return std::string(buffer, received);
}

int Client::recvProtocol(int s, Protocol& protocol) {
    string command;
    char c;
    // MAN PAGE: https://www.man7.org/linux/man-pages/man2/recv.2.html
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
    // MAN PAGE: https://en.cppreference.com/w/cpp/error/invalid_argument
    } catch (const std::invalid_argument& e) {
        std::cout << "Invalid argument: " << lengthStr << "\n" << protocol.length << "\n" << protocol.command << std::endl;
    // MAN PAGE: https://en.cppreference.com/w/cpp/error/out_of_range
   
    } catch (const std::out_of_range& e) {
        // MAN PAGE; https://en.cppreference.com/w/cpp/error/exception/what
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

// handle REPL COMMANDS - not very modular, we are sorry... 
void Client::handleCommand(const std::string &commandInput, int sockfd) {
    std::string command = trimSpace(commandInput);
    // EXIT
    if (command == "exit") {
	// send the command to the server
        sendCommand(sockfd, "exit", "");
	exit(0);
        return;
    }

    // CD
    if (command.substr(0, 2) == "cd") {
        std::string path = command.size() > 3 ? command.substr(3) : "";
        if (!sendCommand(sockfd, "cd", path)) {
            std::cerr << "Error sending cd command." << std::endl;
        }
		Protocol recPac;
		recvProtocol(this->socketFD, recPac);
		cout << recPac.data << endl;

	// PWD
    } else if (command.substr(0, 3) == "pwd") {
        if (!sendCommand(sockfd, "pwd", "")) {
            std::cerr << "Error sending pwd command." << std::endl;
        }
		Protocol recPac;
		recvProtocol(this->socketFD, recPac);
		cout << recPac.data << endl;

	// LS
    } else if (command.substr(0, 3) == "ls") {
        std::string path = command.size() > 3 ? command.substr(3) : "";
        if (!sendCommand(sockfd, "ls", path)) {
            std::cerr << "Error sending ls command." << std::endl;
        }
		Protocol recPac;
		recvProtocol(this->socketFD, recPac);
		cout << recPac.data << endl;

	// MKDIR
    } else if (command.substr(0, 5) == "mkdir") {
        std::string path = command.size() > 6 ? command.substr(6) : "";
        if (!sendCommand(sockfd, "mkdir", path)) {
             std::cerr << "Error sending mkdir command to server." << std::endl;
             return;
        }
   	     Protocol recPac;
    	     recvProtocol(this->socketFD, recPac);
   	     std::cout << recPac.data << std::endl;
	    
	// PUT
	} else if (command.substr(0, 6) == "put -R") {
	        // MAN PAGE: https://en.cppreference.com/w/cpp/io/basic_stringstream
		istringstream stream(command);
		string token, localPath, remotePath;
		stream >> token;
		stream >> token;
		stream >> localPath;
		stream >> remotePath;
		
		putR(sockfd, localPath, localPath, remotePath);
		
	} else if (command.substr(0, 3) == "put") {
	    size_t spaceIndex = command.find(" ", 4);
	    std::string localPath = toPath(command.substr(4, spaceIndex - 4));
	    std::string remotePath = command.size() > spaceIndex + 1 ? command.substr(spaceIndex + 1) : "";
	
		put(sockfd, localPath, remotePath);


	    
	// GET
	} else if (command.substr(0, 6) == "get -R") {
		istringstream stream(command);
		string token, localPath, remotePath;
		stream >> token;
		stream >> token;
		stream >> remotePath;
		stream >> localPath;
	
		
		string remotePathName = getFileName(remotePath);
		
		string localRoot;
		// file/dir will be in current directory and named after the remote
		if(localPath == "" || localPath == "." || localPath == "./") {
			localRoot = clientDir.string();
		} 
		// will be named after remote file
		else if(endsWith(localPath, "/")) {
			// root path
			if(startsWith(localPath, "/")) {
				localRoot = localPath;
			} 
			// assumes its relative dir
			else {
				if(startsWith(localPath, "./")) {
					localPath = removePrefix(localPath, "./");
				}
				localRoot = clientDir.string() + "/" + localPath;
			}
			localRoot.pop_back();
		}
		// root path w/ a name
		else if(startsWith(localPath, "/")) {
			localRoot = localPath;
		}
		// assumes its a relative path w/ a name
		else {
			if(startsWith(localPath, "./")) {
				localPath = removePrefix(localPath, "./");
			}
			localRoot = clientDir.string();
		}
		cout << "actual path: " << localRoot << endl;
		
		// request file from server
		if (!sendCommand(sockfd, "get " + remotePath, "")) {
			std::cerr << "Error sending get command to server." << std::endl;
			return;
		}
		
		sendCommand(sockfd, "get -R " + remotePath, "");	

		while(true) {
			Protocol recPac;
			recvProtocol(this->socketFD, recPac);
			if(recPac.command == "finished") {break;}
			string response = recPac.command;
			istringstream stream(response);
			string type, path;
			stream >> type;
			stream >> path;
										
			
			if(!exists(localRoot + path)) {
				if(type == "-f") {
					cout << "File recieved " << localRoot + path << endl;
					// source: https://www.geeksforgeeks.org/file-system-library-in-cpp-17/
					ofstream file(localRoot + path); 
					if (file.is_open()) { 
						// Write data to the file 
						file << recPac.data; 
						file.close(); 
						
					} else { 
						// Handle the case if any error occured 
						cerr << "Failed to create file: " << localRoot + path 
							 << endl; 
					}
				} else if(type == "-d") {
					cout << "Directory recieved: " << localRoot + path << endl;
					
					create_directory(localRoot + path);
				} else { cout << "something went wrong " << endl; }
			} else {
				cout << "dir alr exists: " << localRoot + path << endl;
			}
			sendCommand(sockfd, "next", "");
		}
		
			
	} else if (command.substr(0, 3) == "get") {
	        // MAN PAGE: https://en.cppreference.com/w/cpp/io/basic_stringstream
		istringstream stream(command);
		string token, localPath, remotePath;
		stream >> token;
		stream >> remotePath;
		stream >> localPath;
		
		cout << "remote path: " << remotePath << endl;
		cout << "local path: " << localPath << endl;

		string remotePathName = getFileName(remotePath);
		
		string localRoot;
		// file/dir will be in current directory and named after the remote
		if(localPath == "" || localPath == "." || localPath == "./") {
			localRoot = clientDir.string() + "/" + remotePathName;
		} 
		// will be named after remote file
		else if(endsWith(localPath, "/")) {
			// root path
			if(startsWith(localPath, "/")) {
				localRoot = localPath + remotePathName;
			} 
			// assumes its relative dir
			else {
				if(startsWith(localPath, "./")) {
					localPath = removePrefix(localPath, "./");
				}
				localRoot = clientDir.string() + "/" + localPath + remotePathName;
			}
		}
		// root path w/ a name
		else if(startsWith(localPath, "/")) {
			localRoot = localPath;
		}
		// assumes its a relative path w/ a name
		else {
			if(startsWith(localPath, "./")) {
				localPath = removePrefix(localPath, "./");
			}
			localRoot = clientDir.string() + "/" + localPath;
		}
		cout << "actual path: " << localRoot << endl;
		
		// request file from server
		if (!sendCommand(sockfd, "get " + remotePath, "")) {
			std::cerr << "Error sending get command to server." << std::endl;
			return;
		}

		Protocol recPac;
		recvProtocol(this->socketFD, recPac);
		
		cout << "command: " << recPac.command << endl;
		cout << "data: " << recPac.data << endl;

	        // MAN PAGE: https://en.cppreference.com/w/cpp/io/basic_stringstream
		istringstream streamCommand(recPac.command);
		string type, remoteThing;
		streamCommand >> type;
		streamCommand >> remoteThing;
		
		// make sure nothing is being overwritten
		if(!exists(localRoot)) {
			// is a directory
			if(type == "-d") {
				create_directory(localRoot);
				cout << "Successfully created directory: " << localRoot << endl;
			}
			// is a file
			else if(type == "-f") {
				ofstream file(localRoot);
				if (file.is_open()) { 
					// write data to the file 
					file << recPac.data; 
					file.close(); 
					cout << "Successfully created file: " << localRoot << endl;
				} 
			}
		}
		//get(sockfd, localPath, remotePath);

	// LCD
    } else if (command.substr(0, 3) == "lcd") {
        // MAN PAGE: https://en.cppreference.com/w/cpp/string/basic_string
	// MAN PAGE: https://en.cppreference.com/w/c/program/getenv
        std::string path = command.size() > 4 ? command.substr(4) : getenv("HOME");
	// MAN PAGE: https://www.man7.org/linux/man-pages/man2/chdir.2.html
        if (chdir(path.c_str()) != 0) {
            std::cerr << "Error changing local directory: " << strerror(errno) << std::endl;
        }

	// LLS
    } else if (command.substr(0, 3) == "lls") {
        std::string path = command.size() > 4 ? command.substr(4) : ".";
	// MAN PAGE: https://www.man7.org/linux/man-pages/man3/opendir.3.html
        DIR* dir = opendir(path.c_str());
        if (dir) {
	    // MAN PAGE: https://man7.org/linux/man-pages/man0/dirent.h.0p.html
            struct dirent* entry;
	    // MAN PAGE: https://www.man7.org/linux/man-pages/man3/readdir.3.html
	    // readdir function returns a pointer to a direent struct to represent the next directory entry in directory stream
            while ((entry = readdir(dir)) != nullptr) {
		// https://www.man7.org/linux/man-pages/man3/readdir.3.html
		// d_name - null terminated filename
                std::cout << entry->d_name << std::endl;
            }
            closedir(dir);
        } else {
            std::cerr << "Error listing local directory." << std::endl;
        }

	// LMKDIR
    } else if (command.substr(0, 6) == "lmkdir") {
        std::string path = command.substr(7);
	// MAN PAGE: https://www.man7.org/linux/man-pages/man2/mkdir.2.html
	// 0755 specifies the permissions for the new directory
        if (mkdir(path.c_str(), 0755) != 0) {
            std::cerr << "Error creating local directory: " << strerror(errno) << std::endl;
        }
	// LPWD
    } else if (command.substr(0, 4) == "lpwd") {
        // attempt to get current working directory 
	char cwd[1024]; // character array to store path of working directory.
	// MAN PAGE: https://www.man7.org/linux/man-pages/man3/getcwd.3.html
        if (getcwd(cwd, sizeof(cwd)) != nullptr) {
            std::cout << cwd << std::endl;
        } else {
            std::cerr << "Error getting local working directory." << std::endl;
        }


	    // HELP
	} else if (command.substr(0, 4) == "help") {
	    // list the commands for the user
	    listCommands();
		
	// if doesn't know - error
    } else {
        std::cerr << "Unknown command." << std::endl;
    }
}

bool Client::startsWith(const std::string& str, const std::string& prefix) { 
	if (str.size() < prefix.size()) {
		return false; 
	}
	// MAN PAGE: https://en.cppreference.com/w/cpp/string/basic_string/compare
	return (str.compare(0, prefix.size(), prefix) == 0); 
}

void Client::listCommands() {
	cout << "\n\nList of Commands:" << endl;
	cout << "=====================================================================" << endl;
	cout << "exit" << endl;
	cout << "	Exits the program." << endl;
	cout << "cd [path]" << endl;
	cout << "	Changes the remote directory to the given path." << endl;
	cout << "get [-R] [remote-path] [local-path]" << endl;
	cout << "	Retrieves the remote-path and stores it on the local machine." << endl;
	cout << "	If local-path isn't specified, it will have the same name as the remote-path." << endl;
	cout << "	-R flag is optional. If specified, files will be copied recursively." << endl;
	cout << "help" << endl;
	cout << "	Displays help text." << endl;
	cout << "lcd [path]" << endl;
	cout << "	Changes local directory to the given path." << endl;
	cout << "	If path is not specified, then changes directory to the local user’s home directory." << endl;
	cout << "lls [path]" << endl;
	cout << "	Display local directory listing of path." << endl;
	cout << "	Displays local directory listing of current directory if path isn't specified." << endl;
	cout << "lmkdir path" << endl;
	cout << "	Creates a local directory specified by path." << endl;
	cout << "lpwd" << endl;
	cout << "	Prints the local working directory." << endl;
	cout << "ls [path]" << endl;
	cout << "	Display a remote directory listing of path." << endl;
	cout << "	Displays a remote directory listing of current directory if path isn't specified." << endl;
	cout << "mkdir path" << endl;
	cout << "	Creates a remote directory specified by path." << endl;
	cout << "put [-R] [local-path] [remote-path]" << endl;
	cout << "	Uploads local-path and stores it on the remote machine." << endl;
	cout << "	If the remote path name is not specified, it will have the same name as the local-path." << endl;
	cout << "	-R flag is optional. If specified, files will be copied recursively." << endl;
	cout << "pwd" << endl;
	cout << "	Displays the remote working directory." << endl;
	cout << "=====================================================================" << endl;
}
