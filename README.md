# CPSC328-Protocol-Project

**Network Programming ReadMe**

Authors:			Shannen Cawley, Dylan Derstine, Kylee Hager

Creation Date:			12/03/24

Last Modified:			12/09/24

Due Date:		    	December 12, 2024

Course: 		   	CPSC 328 Network Programming

Professor Name:			Dr. Schwesinger

Assignment:		 	Network Program Implementation

Purpose:	    		To develop a file server application similar to the sftp program that can serve multiple clients at once.

**How to build the server and client:**
  - Begin in the final-project directory.
  - Type “make”.
  - Both fileclient and fileserver applications will be made
  - Type “./fileclient -h [host] -p [port]” to start the client with [host] being the server’s host name and [port] being the port number to connect to.
  - Type “./fileserver -p [port] -d [directory]” to start the server with [port] being the port number to start the server on and [directory] being the directory to serve files from.

**Purpose of files: **
  - fileClient.cpp: Holds all function definitions needed for the client.
  - fileClient.h: Holds the function signatures needed for the client.
  - main.cpp (in client folder): Calls the functions from fileClient.h in order to make the client usable.
  - Makefile (in client folder): Compiles the code in the client folder to make an executable named “fileclient.”
  - server.cpp: Holds all function definitions needed for the client.
  - server.hpp: Holds all function signatures needed for the server.
  - main.cpp (in server folder): Calls the functions from server.hpp in order to make the server usable.
  - Makefile (in server folder): Compiles the code in the server folder to make an executable named “filesever.”

**Responsibility List:**
  - Shannen Cawley: Mainly fileclient, documentation, helped with readMe.
  - Dylan Derstine: Mainly fileserver, helped with fileclient, documentation.
  - Kylee Hager: Mainly library and documentation, helped with fileclient and readMe

**Protocol:**
	Our protocol contains three sections, each separated by a new line, ‘\n.’ None of these sections have headers, only the information required of each. The first section is the command or main message. This is where the client sends the server the user’s command and the server sends its OK or failure message. The second section is the length of the data, given in ASCII numbers. Both the first and second section cannot contain any new lines (\n) except one at the end of the section, otherwise there will be issues with sending and receiving. The final section is the bulk of the data to send through, typically being the contents of a file to send. The length of this data is given in the second section. Here is the outline:
	[Command]\n
	[Length]\n
	[Data]\n
 
**Assumptions:**
	It is assumed that the user knows the host and port numbers for the server they want to connect to when using the client. The user should also be able to realize that entering “help” will give them a list of commands they can perform.

**Development Process:**
	We started with a design document that went over our basic approach, including our first rendition of our protocol. It was modified slightly before moving on to implementation. The first version of the server was made first, which included the sending and receiving functions with a new version of the protocol. This version got rid of the labels before each section. The client was made shortly after using the same send/receive as the server. From there, the two were updated simultaneously, adding the functions to handle each client command one-by-one, testing throughout. The client dealt with anything it could handle on its own (without the server), and the server was “brought in” for the rest.

**Status:** Majority of the command line arguments were as anticipated. A couple bugs with the get and put requests. 
