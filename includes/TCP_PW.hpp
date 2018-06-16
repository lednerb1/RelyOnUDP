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
#include <vector>
#include <utility>

#define ACK (1 << 0)
#define SYN (1 << 1)
#define FIN (1 << 2)
#define COF (1 << 3)

const int TCP_PW_CLIENT = 1;
const int TCP_PW_SERVER = 2;

extern int MTU, MSS;
extern double timeout;

/*
    Pacote que sera enviado pelo Servidor e Cliente
*/
struct Pacote {
private:
    struct in_addr IP_origem, IP_destino;
    int PORT_origem, PORT_destino;
    int n_ACK, n_SEQ;
    unsigned short flag;
    char dados[1500];
public:
    Pacote();
    Pacote(struct in_addr IP_origem, struct in_addr IP_destino, int PORT_origem, int PORT_destino,
           int n_ACK, int n_SEQ, unsigned short flag, char const  *dados);

    char * getDados();
    struct in_addr getIpOrigem();
    struct in_addr getIpDest();
    int getPortOrigem();
    int getPortDest();
    unsigned short getFlag();
    int getACK();
    void setACK(int n_ACK);
    int getSEQ();
    void setSEQ(int n_SEQ);

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

    int handC;

    /* Cliente */
    struct sockaddr_in C_address;
    int sock = 0, C_valread;
    struct sockaddr_in serv_addr;
    char IP[20];
	int n_SEQ, n_ACK;

    /* Servidor */
    int server_fd, new_socket, S_valread;
    struct sockaddr_in S_address;
    int opt = 1;
    int addrlen;
public:
    TCP_PW(int tipo);

	int timeHandler(clock_t s, clock_t f);

    std::pair<std::pair<int, int>, std::pair<Pacote *, struct sockaddr_in> > recvUDP();
    int sendA(unsigned short flag, struct sockaddr_in dest);

    //Servidor
    int start(int argc, char const *argv[]);
    void listen();

    //Cliente
    int connectA();
    void handShake();
    void disconnect();
    void sendMsg(char const *text);

    struct sockaddr_in getServerAddr();
    struct sockaddr_in *getServerAddrPtr();
    int getSock();
};

#endif
