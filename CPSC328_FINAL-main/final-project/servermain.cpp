/************************************************************/
/* Author: Shannen Cawley, Kylee Hager, Dylan Derstine      */
/* Creation Date: November 25, 2024                         */
/* Due Date: December 12, 2024                              */
/* Course: CPSC 328                                         */
/* Professor Name: Dr. Schwesinger                          */
/* Assignment: 7                                            */
/* Filename: servermain.cpp                                 */
/* Purpose: This file contains the main function for        */
/*          initializing the server. It parses command line */
/*          arguments and starts the server.                */
/************************************************************/

#include <iostream>
#include <getopt.h>

#include "fileserver.h"

using namespace std;

//Struct to hold parsed data needed by the server.
struct options {
    string port;
	string dir;
};

options parse_args(int argc, char** argv);

int main(int argc, char* argv[]) {
	options args = parse_args(argc, argv);
	
	// create Server object
	Server server = Server(args.port, args.dir);
	
	cout << "Hello World!\n";
    return 0;
}

// code taken from schwesinger in the 235 project
/*
* Description:	Parses command line arguments needed by the server.
* Parameters:	int argc: the number of arguments
*		char** argv: the arguments
* Return:	A struct containing the parsed arguments.
*/
options parse_args(int argc, char** argv) {
    struct options result = {
        .port = "", 
        .dir = "",
    };
    int opt;

    while( (opt = getopt(argc,argv,"p:d:")) != -1 ){
        switch(opt){
        case 'p':
            result.port = optarg;
            break;
        case 'd':
            result.dir = optarg;
            break;
        default:
            cout << "-p for port and -d for dir" << endl;
            exit(EXIT_FAILURE);
        }
    }
	
	// Ensure that all required command line arguments were specified
    if (result.port == "" || result.dir == "") {
        cout << "ERROR: Missing required command line argument" << endl;
        cout << "-p for port and -d for dir" << endl;
        exit(EXIT_FAILURE);
    }

    return result;
}
