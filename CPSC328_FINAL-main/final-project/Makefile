CFLAGS = -g -Wall --std=c++17

all: fileserver fileclient

fileserver: servermain.o fileserver.o commands.o helpers.o
	g++ $(CFLAGS) -o fileserver servermain.o fileserver.o commands.o helpers.o -lstdc++fs

servermain.o: servermain.cpp fileserver.h
	g++ $(CFLAGS) -c servermain.cpp

fileserver.o: fileserver.cpp fileserver.h
	g++ $(CFLAGS) -c fileserver.cpp
	
commands.o: commands.cpp fileserver.h
	g++ $(CFLAGS) -c commands.cpp
	
helpers.o: helpers.cpp fileserver.h
	g++ $(CFLAGS) -c helpers.cpp

fileclient: clientmain.o fileclient.o
	g++ $(CFLAGS) -o fileclient clientmain.o fileclient.o -lstdc++fs

clientmain.o: clientmain.cpp fileclient.h
	g++ $(CFLAGS) -c clientmain.cpp

fileclient.o: fileclient.cpp fileclient.h
	g++ $(CFLAGS) -c fileclient.cpp

clean:
	rm -f fileserver fileclient *.o

.PHONY: all clean
