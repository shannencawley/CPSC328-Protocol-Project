#ifndef FILECLIENT_H
#define FILECLIENT_H

#include <string>
#include <filesystem>
#include <iostream>
using namespace std;

/*
 * Struct to hold information sent in a packet following the protocol
*/
struct Protocol {
	string command="";
	int length=0;
	string data="";
};

class Client {
	
	private:
		//the current working directory's path
		std::filesystem::path clientDir;
		//the socket file descriptor of the client
		int socketFD;

		/*
		* Description:	Trims out the whitespace in a string
		* Parameters:	string &str: the string to trim
		* Return:	The trimmed string.
		*/
		std::string trimSpace(const std::string &str);

		/*
		* Description:	Checks to see is a string starts with a certain prefix.
		* Parameters:	string &str: the string you're checking.
		*		string &prefix: the prefix to look for in str.
		* Return:	True if the prefix is in str, false if not.
		*/
		bool startsWith(const std::string& str, const std::string& prefix);

		/*
		* Description:	Lists all viable commands.
		* Parameters:	none.
		* Return:	none.
		*/
		void listCommands();
	
	public:
		// from Dr. Schwesinger
		/*
		* Description:	Sends a char* message to a server.
		* Parameters:	int sockfd: the file descriptor
		*		char* buf: the message to send
		*		int len: the length of the message
		* Return:	-1 on failure, number of bytes sent on success
		*/
		int reallySend(int sockfd, const char* buf, int len);

		/*
		* Description:	Receives a char* message from a server.
		* Parameters:	int sockfd: the file descriptor
		*		char* buf: where the message will be put
		*		int len: the length of the message
		* Return:	Total number of bytes recieved.
		*/
		int reallyRecv(int sockfd, char* buf, int len);

		/*
		* Description:	Connects the client to the server.
		* Parameters:	string &host: the hostname of the server to connect to
		*		string &port: the port number to connect to
		* Return:	The client's file descriptor.
		*/
		int connectToServer(const std::string &host, const std::string &port);

		/*
		* Description:	Sends a command to a server (using protocol).
		* Parameters:	int sockfd: the file descriptor
		*		string &command: the command to send
		*		string &data: the data to send
		* Return:	True if all data was sent, false if error occurs.
		*/
		bool sendCommand(int sockfd, const std::string &command, const std::string &data = "");

		/*
		* Description:	Receives from a server (using protocol).
		* Parameters:	int sockfd: the file descriptor
		* Return:	The server's message
		*/
		std::string receiveResponse(int sockfd);
		
		/*
		* Description:	Receives a command from a server (using protocol).
		* Parameters:	int s: the file descriptor
		*		Protocol &protocol: the struct containing the recieved data
		* Return:	1 upon success.
		*/
		int recvProtocol(int s, Protocol& protocol);

		/*
		* Description:	Handles a single command.
		* Parameters:	string &command: the command requested by the user
		*		int sockfd: the client's file descriptor
		* Return:	none.
		*/
		void handleCommand(const std::string &command, int sockfd);

		/*
		* Description:	Gets a file from the server and puts it on the client.
		* Parameters:	int sockfd: the client's file descriptor
  		*		string localPath: the file's path on the client's side
      		*		string remotePath: the file's path on the server's side
		* Return:	none.
		*/
		void get(int sockfd, string localPath, string remotePath);

		/*
		* Description:	Gets a file from the server and puts it on the client recursively.
		* Parameters:	int sockfd: the client's file descriptor
    		*		string oglocalPath: the file's original path on the client's side
  		*		string localPath: the file's new path on the client's side
      		*		string remotePath: the file's path on the server's side
		* Return:	none.
		*/
		void getR(int sockfd, string ogLocalPath, string localPath, string remotePath);

		/*
		* Description:	Puts a file from the client and onto the server recursively.
		* Parameters:	int sockfd: the client's file descriptor
    		*		string oglocalPath: the file's original path on the client's side
  		*		string localPath: the file's new path on the client's side
      		*		string remotePath: the file's path on the server's side
		* Return:	none.
		*/
		void putR(int sockfd, string ogLocalPath, string localPath, string remotePath);

		/*
		* Description:	Puts a file from the client onto the server.
		* Parameters:	int sockfd: the client's file descriptor
  		*		string localPath: the file's path on the client's side
      		*		string remotePath: the file's path on the server's side
		* Return:	none.
		*/
		void put(int sockfd, string localPath, string remotePath);
};

#endif
