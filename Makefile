CC = g++
LINKS = -std=c++11 -O2
INCLUDES = includes/TCP_PW.cpp

all:
	$(CC) -o Server server.cpp $(INCLUDES) $(LINKS)
	$(CC) -o Client client.cpp $(INCLUDES) $(LINKS)

server:
	$(CC) -o Server server.cpp $(INCLUDES) $(LINKS)

client:
	$(CC) -o Client client.cpp $(INCLUDES) $(LINKS)

.PHONY: all server client
