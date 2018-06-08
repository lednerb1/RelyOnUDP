#include "TCP_PW.hpp"

int MTU;

Pacote::Pacote(){
    struct in_addr a;
    Pacote(a, a, 8080, 8080, 0, 0, 0, "");
}

Pacote::Pacote(struct in_addr IP_origem, struct in_addr IP_destino, int PORT_origem, int PORT_destino,
    int n_ACK, int n_SEQ, int flag, char const  *dados){
    this -> IP_origem = IP_origem;
    this -> IP_destino = IP_destino;
    this -> PORT_origem = PORT_origem;
    this -> PORT_destino = PORT_destino;
    this -> n_ACK = n_ACK;
    this -> n_SEQ = n_SEQ;
    this -> flag = flag;
    strcpy(this -> dados, dados);
    // printf("Pacotin criado com a msg: %s\n", this -> dados);
}

char * Pacote::getDados(){
    return this -> dados;
}

struct in_addr Pacote::getIpOrigem(){
    return this -> IP_origem;
}
struct in_addr Pacote::getIpDest(){
    return this -> IP_destino;
}
int Pacote::getPortOrigem(){
    return this -> PORT_origem;
}
int Pacote::getPortDest(){
    return this -> PORT_destino;
}

int Pacote::getFlag(){
    return this -> flag;
}

TCP_PW::TCP_PW(int tipo){
    this -> tipo = tipo;
}


/* Client */
void TCP_PW::handShake(){
    sendA("", SYN, this -> getServerAddr());
    while(1){
        InfoRetRecv ret = InfoRetRecv(recvUDP());
        if(ret.s == 0){
            if(ret.buff -> getFlag() & ACK && ret.buff -> getFlag() & SYN){
                printf("Recebido ACK-SYN\n");
                sendA("", ACK, ret.peer_addr);
                break;
            }
        }
    }

    MTU = 1500;
}

void TCP_PW::disconnect(){
    sendA("", FIN, this -> getServerAddr());
    while(1){
        InfoRetRecv ret = InfoRetRecv(recvUDP());
        if(ret.s == 0){
            if(ret.buff -> getFlag() & ACK && ret.buff -> getFlag() & FIN){
                printf("Recebido ACK-FIN\n");
                sendA("", ACK, ret.peer_addr);
                break;
            }
        }
    }
}

int TCP_PW::start(int argc, char const *argv[]){
    for(int i = 1; i < argc; i++){
        if(strcmp(argv[i], "-i") == 0){
            strcpy(this -> IP, argv[i + 1]);
        } else if(strcmp(argv[i], "-p") == 0){
            this -> PORT = atoi(argv[i + 1]);
        }
    }

    if(this -> tipo == TCP_PW_SERVER){
        if ((this -> sock = socket(AF_INET, SOCK_DGRAM, 0)) == 0){
            perror("socket failed");
            return -1;
        }

        this -> S_address.sin_family = AF_INET;
        this -> S_address.sin_addr.s_addr = INADDR_ANY;
        this -> S_address.sin_port = htons(this -> PORT);

        // Forcefully attaching socket to the port
        if (bind(this -> sock, (struct sockaddr *)&(this -> S_address), sizeof(this -> S_address)) < 0){
            perror("bind failed");
            return -1;
        }
    } else {
        if ((this -> sock = socket(AF_INET, SOCK_DGRAM, 0)) <= 0){
            printf("\n Socket creation error \n");
            return -1;
        }

        // memset(&(this -> serv_addr), '0', sizeof(this -> serv_addr));

        this -> C_address.sin_family = AF_INET;
        this -> C_address.sin_addr.s_addr = INADDR_ANY;
        this -> C_address.sin_port = htons(0);

        // if(getsockname(this -> sock, (struct sockaddr *)&(this -> C_address), &len_inet)){
        //     printf("Creem\n");
        // }

        this -> serv_addr.sin_family = AF_INET;
        this -> serv_addr.sin_addr.s_addr = inet_addr(this -> IP);
        this -> serv_addr.sin_port = htons(this -> PORT);
        //
        // // Forcefully attaching socket to the port
        // if (bind(this -> sock, (struct sockaddr *)&(this -> serv_addr), sizeof(this -> serv_addr)) < 0){
        //     perror("bind failed");
        //     return -1;
        // }
    }
}

int TCP_PW::connectA(){
    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, this -> IP, &(this -> serv_addr).sin_addr) <= 0){
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(this -> sock, (struct sockaddr *)&(this -> serv_addr), sizeof(this -> serv_addr)) < 0){
        printf("\nConnection Failed\n");
        return -1;
    }

    handShake();
}

int TCP_PW::sendA(char const *text, int flag, sockaddr_in dest){
    Pacote *pct;
    if(this -> tipo == TCP_PW_CLIENT){
        pct = new Pacote(this -> C_address.sin_addr, dest.sin_addr, this -> C_address.sin_port, dest.sin_port, 0, 0, flag, text);
    } else {
        pct = new Pacote(this -> S_address.sin_addr, dest.sin_addr, this -> S_address.sin_port, dest.sin_port, 0, 0, flag, text);
    }
    // std::cout << "Enviando para " << pct -> getIpDest().s_addr << " a partir de " << pct -> getIpOrigem().s_addr << " para a porta " << ntohs(pct -> getPortDest()) << " a partir da porta " << ntohs(pct -> getPortOrigem()) << std::endl;
    sendto(this -> getSock(), pct, sizeof(Pacote), 0, (sockaddr *)&dest, sizeof(dest));
}

void TCP_PW::sendMsg(char const *text){
    Pacote *pct = new Pacote(this -> C_address.sin_addr, this -> getServerAddr().sin_addr, this -> C_address.sin_port, this -> getServerAddr().sin_port, 0, 0, ACK, text);
    int tamPct = sizeof(Pacote) + strlen(pct -> getDados());
    sendto(this -> getSock(), pct, sizeof(Pacote), 0, (sockaddr *)this -> getServerAddrPtr(), sizeof(this -> getServerAddr()));
}

std::pair<std::pair<int, int>, std::pair<Pacote *, struct sockaddr_in> > TCP_PW::recvUDP(){
    Pacote *buff = new Pacote();
    struct sockaddr_in peer_addr;
    socklen_t peer_addr_len;
    peer_addr_len = sizeof(struct sockaddr_storage);

    int nread = recvfrom(this -> getSock(), buff, sizeof(Pacote), 0, (struct sockaddr *)&peer_addr, &peer_addr_len);
    if(nread == -1) return {{-1, -1}, {NULL, peer_addr}};
    char host[NI_MAXHOST], service[NI_MAXSERV];
    int s = getnameinfo((struct sockaddr *) &peer_addr, peer_addr_len, host, NI_MAXHOST, service, NI_MAXSERV, NI_NUMERICSERV);
    return {{s, nread}, {buff, peer_addr}};
}

void TCP_PW::listen(){
    int hand = 0, disc = 0;
    while(1){
        InfoRetRecv ret = InfoRetRecv(recvUDP());
        if(ret.s == 0){
            /* HANDSHAKE */
            if(ret.buff -> getFlag() & SYN){
                printf("Recebido SYN\n");
                sendA("", ACK | SYN, ret.peer_addr);
                hand = 1;
                continue;
            }

            /* DISCONNECT */
            if(ret.buff -> getFlag() & FIN){
                printf("Recebido FIN\n");
                sendA("", FIN | ACK, ret.peer_addr);
                disc = 1;
                continue;
            }

            /* Recebeu um ACK */
            if(ret.buff -> getFlag() & ACK){
                printf("Recebido ACK\n");
                /* Se for um ACK para confirmar a conexão */
                if(hand){
                    printf("Conexao estabelecida\n");
                    hand = 0;
                } else if(disc){ /* Se for um ACK para confirmar o encerramento da conexão */
                    printf("Conexao encerrada\n");
                    disc = 0;
                } else {
                    printf("Mensagem recebida: %s\n", ret.buff -> getDados());
                    sendA("", ACK, ret.peer_addr);
                }
            }

            //printf("Recebido %ld bytes de %s:%s\nMensagem: %s\n\n", (long)nread, host, service, buff -> getDados());
        } else {
            fprintf(stderr, "getnameinfo: %s\n", gai_strerror(ret.s));
        }
    }
}

struct sockaddr_in TCP_PW::getServerAddr(){
    return this -> serv_addr;
}

struct sockaddr_in * TCP_PW::getServerAddrPtr(){
    return &(this -> serv_addr);
}

int TCP_PW::getSock(){
    return this -> sock;
}
