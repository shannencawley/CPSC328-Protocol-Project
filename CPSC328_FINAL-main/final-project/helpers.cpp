/************************************************************/
/* Author: Dylan Derstine, Shannen Cawley, Kylee Hager,     */
/* Creation Date: November 25, 2024                         */
/* Due Date: December 12, 2024                              */
/* Course: CPSC 328                                         */
/* Professor Name: Dr. Schwesinger                          */
/* Assignment: 7                                            */
/* Filename: helpers.cpp                                   */
/* Purpose: This file contains utility functions for the    */
/*          server, including file validation and           */
/*          manipulation functions for server/client communication. */
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

// list of functions that id basically have in python

// src: https://davekb.com/browse_programming_tips:cpp_string_starts_with:txt
bool Server::startsWith(const std::string &str, const std::string &target) {
	return str.rfind(target) == 0;
}

// removes prefix on two of the same strings or wtvr
string Server::removePrefix(string str1, string prefix) {
	str1.erase(0, prefix.length());
 
    return str1;
}

// makes sure file exists, is a file, and within bounds
bool Server::isValidFile(path file) {
	if(exists(file) && !is_directory(file)) {
		// make sure its within bounds
		path canonicalPath = canonical(file);
		if(startsWith(canonicalPath, serverDir.string())) {
			// is within bounds
			return true;
		}
	}
	return false;
}

// same as above
bool Server::isValidDir(path dir) {
	if(exists(dir) && is_directory(dir)) {
		// make sure its within bounds
		path canonicalPath = canonical(dir);
		if(startsWith(canonicalPath, serverDir.string())) {
			// is within bounds
			return true;
		}
	}
	return false;
}

// converts a root path to what the client should see
// no error checking, should be done elsewhere
string Server::toClientPath(path dir) {
	string clientPath = removePrefix(dir.string(), serverDir.string());
	
	// incase it's at the root directory
	clientPath==""?clientPath="/":clientPath=clientPath;
	
	return clientPath;
}

string Server::readFile(const string& dir) {
    ifstream f(dir);

    // Check if the file is successfully opened
    if (!f.is_open()) {
        cerr << "Error opening the file!";
        return "";
    }

    // String variable to store the read data
    stringstream file;
    string s;

    // Read each line of the file and append it to the string stream
    while (getline(f, s))
        file << s << "\n";

    // Close the file
    f.close();

    return file.str();
}

/*path clientPathToRoot(path clientDir, path dir) {
	// 
	if(dir == ".") {
		
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
}*/
