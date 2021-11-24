
all: echo-client echo-server

echo-client: echo-client.o
	g++ -o echo-client echo-client.o -pthread

echo-client.o: echo-client.cpp
	g++ -c -o echo-client.o echo-client.cpp

echo-server: echo-server.o
	g++ -o echo-server echo-server.o -pthread

echo-server.o: echo-server.cpp
	g++ -c -o echo-server.o echo-server.cpp
clean:
	rm -f echo-client echo-server *.o
