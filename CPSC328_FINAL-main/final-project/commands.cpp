/************************************************************/
/* Author: Dylan Derstine, Shannen Cawley, Kylee Hager     */
/* Creation Date: November 25, 2024                         */
/* Due Date: December 12, 2024                              */
/* Course: CPSC 328                                         */
/* Professor Name: Dr. Schwesinger                          */
/* Assignment: 7                                            */
/* Filename: commands.cpp                                   */
/* Purpose: This file contains the command implementations  */
/*          for the server, including pwd, cd, mkdir, ls,   */
/*          get, and getR commands.                         */
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

void Server::pwd(int client, path clientDir) {

	
	sendResponse(client, "200", toClientPath(clientDir));
}

void Server::cd(int client, path& clientDir, string pathname) {
	bool success = false;
	
	// no path (go to init dir)
	if(pathname == "") {
		clientDir = serverDir;
		success = true;
	}
	// root path
	else if(startsWith(pathname, "/")) {
		string requestedPath = serverDir.string() + pathname;
		// make sure its a valid dir
		if(isValidDir(requestedPath)) {
			clientDir = canonical(path(requestedPath));
			success = true;
		}
	}
	// assumes its a relative path
	else {
		string requestedPath;
		// setup to create root path
		if(startsWith(pathname, "./")) {
			requestedPath = clientDir.string() + "/" + removePrefix(pathname, "./");
		} else {
			requestedPath = clientDir.string() + "/" + pathname;
		}
		
		cout << "path: " << requestedPath << endl;
		
		if(isValidDir(requestedPath)) {
			clientDir = canonical(path(requestedPath));
			success = true;
		}
	}
	
	if(success) {
		sendResponse(client, "200", toClientPath(clientDir));
	} else {
		sendResponse(client, "400", "Directory not found");
	}
}

void Server::mkdir(int client, string data, path clientDir) {
	// needs work
	
	if(exists(clientDir / data)) {
		sendResponse(client, "400", "Directory already exists");
	} else {
		create_directory(clientDir / data);
		sendResponse(client, "200", "Directory successfully created");
	}
}

// part of this code is from example 2 in https://www.geeksforgeeks.org/file-system-library-in-cpp-17/
void Server::ls(int client, path clientDir) {	
	string dirList;

	if (exists(clientDir) 
        && is_directory(clientDir)) { 
        // Loop through each item (file or subdirectory) in 
        // the directory 
		int i = 0;
        for (const auto& entry : 
            filesystem::directory_iterator(clientDir)) { 
			i++;
            // Output the path of the file or subdirectory 
            //cout << "File: " << removePrefix(entry.path(), clientDir.string() + "/") << endl; 
			dirList += removePrefix(entry.path(), clientDir.string() + "/") + "  " + (i%7==0?"\n":"");
        }
		sendResponse(client, "200", dirList);
    } 
    else { 
        // Handle the case where the directory doesn't exist 
        cerr << "Directory not found." << endl; 
    }
}

void Server::get(int client, path clientDir, string dir) {
	// get the root path
	
	// is root
	if(dir[0] == '/') {
		// do nothing..
		dir = dir; // 10x coder
	}
	// assumes its a relative path
	else {
		if(startsWith(dir, "./")) {
			dir = removePrefix(dir, "./");
		}
		dir = clientDir.string() + "/" + dir;
	}
	
	// if invalid send error
	if(!(isValidDir(path(dir)) || isValidFile(path(dir)))) {
		sendResponse(client, "400", "File or Directory was not found");
	}
	
	cout << "Sending file: " << dir << endl;
	
	string clientPath = toClientPath(dir);
	if(is_directory(dir)) {
		sendResponse(client, "-d " + clientPath, "");
	} else {
		sendResponse(client, "-f " + clientPath, readFile(dir));
	}
}

void Server::getR(int client, path clientDir, string dir) {
	// is root
	if(dir[0] == '/') {
		// do nothing..
		dir = dir; // 10x coder
	}
	// assumes its a relative path
	else {
		if(startsWith(dir, "./")) {
			dir = removePrefix(dir, "./");
		}
		dir = clientDir.string() + "/" + dir;
	}
	
	// make sure its valid
	if(isValidDir(path(dir))) {		
		// directory
		for(const auto& entry : directory_iterator(dir)) {
			get(client, clientDir, entry.path().string());
			while(true) {
				Protocol request{};
				if(recvProtocol(client, request) != 1) {
					perror("Request failed");
				}
				if(request.command == "next") {break;}
			}
			
			if(is_directory(entry.path())) {
				getR(client, clientDir, entry.path().string());
			}
		}
	}
}
