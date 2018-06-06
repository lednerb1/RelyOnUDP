#ifndef TCP_PW_HPP
#define TCP_PW_HPP

#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <iostream>
#include <string>
#include <utility>

#define ACK (1 << 0)
#define SYN (1 << 1)
#define FIN (1 << 2)

const int TCP_PW_CLIENT = 1;
const int TCP_PW_SERVER = 2;

/*
    Pacote que sera enviado pelo Servidor e Cliente
*/
struct Pacote {
private:
    char IP_origem[20], IP_destino[20];
    int PORT_origem, PORT_destino;
    int n_ACK, n_SEQ;
    int flag;
    char dados[100];
public:
    Pacote();
    Pacote(char const * IP_origem, char const * IP_destino, int PORT_origem, int PORT_destino,
           int n_ACK, int n_SEQ, int flag, char const  *dados);

    char * getDados();
    char * getIpOrigem();
    int getFlag();
};

struct InfoRetRecv {
    int s, nread;
    Pacote *buff;
    struct sockaddr_in peer_addr;

    InfoRetRecv(std::pair<std::pair<int, int>, std::pair<Pacote *, struct sockaddr_in> > p){
        this -> s = p.first.first;
        this -> nread = p.first.second;
        this -> buff = p.second.first;
        this -> peer_addr = p.second.second;
    }
};

class TCP_PW {
private:
    int tipo;
    int PORT;

    /* Cliente */
    struct sockaddr_in C_address;
    int sock = 0, C_valread;
    struct sockaddr_in serv_addr;
    char IP[20];

    /* Servidor */
    int server_fd, new_socket, S_valread;
    struct sockaddr_in S_address;
    int opt = 1;
    int addrlen;
public:
    TCP_PW(int tipo);

    void handShake();
    void disconnect();
    std::pair<std::pair<int, int>, std::pair<Pacote *, struct sockaddr_in> > recvUDP();

    //Servidor
    int start(int argc, char const *argv[]);
    void listen();

    //Cliente
    int connectA();
    int sendA(char const *text, int flag, struct sockaddr_in dest);

    struct sockaddr_in getServerAddr();
    int getSock();
};

#endif
