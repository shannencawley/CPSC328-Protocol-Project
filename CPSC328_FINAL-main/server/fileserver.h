/************************************************************/
/* Author: Dylan Derstine, Shannen Cawley, Kylee Hager     */
/* Creation Date: November 25, 2024                         */
/* Due Date: December 12, 2024                              */
/* Course: CPSC 328                                         */
/* Professor Name: Dr. Schwesinger                          */
/* Assignment: 7                                            */
/* Filename: fileserver.h                                   */
/* Purpose: This file contains all the server implementations*/
/*          necessary to accept client connections and handle*/
/*          file server operations.                         */
/************************************************************/

#ifndef SERVER 
#define SERVER

#include <iostream>
#include <string>
#include <filesystem>


using namespace std;
using namespace std::filesystem;

/*
 * Struct to hold information sent in a packet following the protocol.
*/
struct Protocol {
	string command="";
	int length=0;
	string data="";
};

class Server {

public:
	/*
	* Description:	Initiates a server.
	* Parameters:	string port: port number to run the server on
	*		string directory: the directory to serve files from
	* Return:	none.
	*/
    Server(string port, string directory);
	
	/*
	* Description:	Gracefully shuts down the server
	* Parameters:	none.
	* Return:	none.
	*/
    ~Server();
private:
	//the current directory of the server
	path serverDir;

	/*
	* Description:	Accepts client connections indefinitely
	* Parameters:	int sock: the file descriptor
	* Return:	none.
	*/
	void run(int sock); // loop

	/*
	* Description:	Receives a command from a client (using protocol).
	* Parameters:	int s: the file descriptor
	*		Protocol &protocol: the struct containing the received data
	* Return:	1 upon success.
	*/
	int recvProtocol(int s, Protocol& protocl);

	/*
	* Description:	Sends all bytes to a client.
	* Parameters:	int s: the file descriptor
	*		string msg: the message to send
	* Return:	-1 on failure, number of bytes sent on success.
	*/
	int sendall(int s, string msg);

	/*
	* Description:	Sends a response to the client using the protocol.
	* Parameters:	int cllient: the client's file descriptor
	*		string status: the status to send to the client (command in protocol)
	*		string message: the message to send (data in protocol)
	* Return:	none.
	*/
	void sendResponse(int client, string status, string message);

	/*
	* Description:	Creates a socket.
	* Parameters:	string port: the port number to create the socket on
	* Return:	The socket file descriptor of the server.
	*/
	int createSocket(string port);

	/*
	* Description:	Handles a client.
	* Parameters:	int client: the client's file descriptor
	* Return:	none.
	*/
	void handleClient(int client);

	/*
	* Description:	Handles a single command given by the client.
	* Parameters:	Protocol protocol: struct containing information sent by client
	*		int client: the client's file descriptor
	*		bool &open: tells if the client's file descriptor is still open
	*		path &clientDir: the client's current directory
	* Return:	none.
	*/
	void handleCommand(Protocol protocol, int client, bool& open, path& clientDir);

	/*
	* Description:	Handles client's ls command. (shows all files in current directory)
	* Parameters:	int client: the client's file descriptor
	*		path path: the path to get files from
	* Return:	none.
	*/
	void ls(int client, filesystem::path path);

	/*
	* Description:	Handles client's cd command. (travels to a new directory)
	* Parameters:	int client: the client's file descriptor
	*		string data: the new directory to change to
	*		path &clientDir: the client's current directory in the server
	* Return:	none.
	*/
	void cd(int client, path& clientDir, string data);

	/*
	* Description:	Handles client's mkdir command. (makes a directory)
	* Parameters:	int client: the client's file descriptor
	*		string dir: name of the new directory to make
	*		path clientDir: the client's current directory in the server
	* Return:	none.
	*/
	void mkdir(int client, string data, path clientDir);

	/*
	* Description:	Handles client's get command.
	*		(Get a file from the server and put it in the client)
	* Parameters:	int client: the client's file descriptor
	*		string dir: the directory to send files from
	*		path clientDir: the client's current directory in the server
	* Return:	none.
	*/
	void get(int client, path clientDir, string dir);

	/*
	* Description:	Handles client's get command recursively.
	* Parameters:	int client: the client's file descriptor
	*		string dir: the directory to send files from
	*		path clientDir: the client's current directory in the server
	* Return:	none.
	*/
	void getR(int client, path clientDir, string dir, bool first);

	/*
	* Description:	Handles client's put command.
	*		(Puts a file from the client onto the server)
	* Parameters:	int client: the client's file descriptor
	*		string command: the command sent by the client
	*		string data: the data from the file  to copy
	*		path clientDir: the client's current directory in the server
	* Return:	none.
	*/
	void put(int client, string command, string data, path clientDir);

	/*
	* Description:	Handles client's pwd command. (shows current working directory)
	* Parameters:	int client: the client's file descriptor
	*		path clientDir: the client's current directory in the server
	* Return:	none.
	*/
	void pwd(int client, path clientDir);

	// all client functions
	/*void exit();
	void cd(string path);
	void get(bool recursive, string path);
	void help();
	void ls(string path);
	void put(bool recursive, string path);
	void pwd();*/
	
	// helper functions
	/*
	* Description:	Checks whether a client's path is valid in the proper directory.
	* Parameters:	path dir: the client's path
	* Return:		True if valid, false if not.
	*/
	bool clientPathIsValid(path dir);

	/*
	* Description:	Removes the prefix from a string.
	* Parameters:	string &str1: the string to remove the prefic from
 	*		string &target: the prefix to remove
	* Return:	The string with the removed prefix.
	*/
	string removePrefix(string str1, string prefix);

	/*
	* Description:	Tells whether a string starts with a specific set of characters.
	* Parameters:	string &str: the string to check for the starting characters
 	*		string &target: the starting characters
	* Return:	True if the string str starts with the target string. False if not.
	*/
	bool startsWith(const std::string &str, const std::string &target);

	/*
	* Description:	Tells whether a file is valid for use by the server.
	* Parameters:	path dir: the path of the file to check
	* Return:	True if the path is usable. False if not.
	*/
	bool isValidFile(path dir);

	/*
	* Description:	Tells whether a directory is valid for use by the server.
	* Parameters:	path dir: the path of the directory to check
	* Return:	True if the path is usable. False if not.
	*/
	bool isValidDir(path dir);

	/*
	* Description:	Converts the root path to what the client should see.
	* Parameters:	path dir: the path to convert
	* Return:	The path for the client's use.
	*/
	string toClientPath(path dir);

	/*
	* Description:	Reads a file's data into a string.
	* Parameters:	string &dir: the file to open and read
	* Return:	The file's data
	*/
	string readFile(const string& dir);
};

#endif
