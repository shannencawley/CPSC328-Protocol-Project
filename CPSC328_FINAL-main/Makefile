all: server

server: main.o server.o
	g++ -o server main.o server.o

main.o: main.cpp server.hpp
	g++ -c main.cpp

server.o: server.cpp server.hpp
	g++ -c server.cpp

clean:
	rm -f server *.o

.PHONY: all clean