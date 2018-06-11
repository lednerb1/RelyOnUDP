#include "TCP_PW.hpp"

int MTU = 1500;
double timeout = 0.05;

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

int TCP_PW::timeHandler(clock_t s, clock_t f){
	double t = (f - s) * 1000.0 / CLOCKS_PER_SEC;
	if(t > timeout){
		printf("Provavel perda de pacote!\n");
		return 1;
	}
	return 0;
}

/* Client */
void TCP_PW::handShake(){
	printf("Enviando SYN\n");
    sendA("", SYN, this -> getServerAddr());
    int cnt = 0;
    clock_t start = clock();
    while(1){
        InfoRetRecv ret = InfoRetRecv(recvUDP());        
        if(ret.s == 0){
            if(ret.buff -> getFlag() & ACK && ret.buff -> getFlag() & SYN){
                printf("Recebido ACK | SYN\n");
                sendA("", ACK, ret.peer_addr);
                printf("Enviando ACK\n");
                break;
            }
        }
        if(timeHandler(start, clock())){
			printf("Enviando SYN\n");
			sendA("", SYN, this -> getServerAddr());
			start = clock();
		}
    }
}

void TCP_PW::disconnect(){
    sendA("", FIN, this -> getServerAddr());
    printf("Enviando FIN\n");
    clock_t start = clock();
    while(1){
        InfoRetRecv ret = InfoRetRecv(recvUDP());
        if(ret.s == 0){
            if(ret.buff -> getFlag() & ACK && ret.buff -> getFlag() & FIN){
                printf("Recebido ACK | FIN\n");
                sendA("", ACK, ret.peer_addr);
                printf("Enviando ACK\n");
                break;
            }
        }
        if(timeHandler(start, clock())){
			printf("Enviando FIN\n");
			sendA("", FIN, this -> getServerAddr());
			start = clock();
		}
    }
}

int TCP_PW::start(int argc, char const *argv[]){
    for(int i = 1; i < argc; i++){
        if(strcmp(argv[i], "-i") == 0){
            strcpy(this -> IP, argv[i + 1]);
        } else if(strcmp(argv[i], "-p") == 0){
            this -> PORT = atoi(argv[i + 1]);
        } else if(strcmp(argv[i], "-MTU") == 0){
			MTU = atoi(argv[i + 1]);
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

        this -> C_address.sin_family = AF_INET;
        this -> C_address.sin_addr.s_addr = INADDR_ANY;
        this -> C_address.sin_port = htons(0);

        // if(getsockname(this -> sock, (struct sockaddr *)&(this -> C_address), &len_inet)){
        //     printf("Creem\n");
        // }

        this -> serv_addr.sin_family = AF_INET;
        this -> serv_addr.sin_addr.s_addr = inet_addr(this -> IP);
        this -> serv_addr.sin_port = htons(this -> PORT);
    }
}

int TCP_PW::connectA(){
    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, this -> IP, &(this -> serv_addr).sin_addr) <= 0){
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }
	
	this -> n_ACK = 0;
	this -> n_SEQ = 0;
    if (connect(this -> sock, (struct sockaddr *)&(this -> serv_addr), sizeof(this -> serv_addr)) < 0){
        printf("\nConnection Failed\n");
        return -1;
    }

    handShake();
}

int TCP_PW::sendA(char const *text, int flag, sockaddr_in dest){
    Pacote *pct;
    if(this -> tipo == TCP_PW_CLIENT){
        pct = new Pacote(this -> C_address.sin_addr, dest.sin_addr, this -> C_address.sin_port, dest.sin_port, this -> n_ACK, this -> n_SEQ, flag, text);
    } else {
        pct = new Pacote(this -> S_address.sin_addr, dest.sin_addr, this -> S_address.sin_port, dest.sin_port, this -> n_ACK, this -> n_SEQ, flag, text);
    }
    // std::cout << "Enviando para " << pct -> getIpDest().s_addr << " a partir de " << pct -> getIpOrigem().s_addr << " para a porta " << ntohs(pct -> getPortDest()) << " a partir da porta " << ntohs(pct -> getPortOrigem()) << std::endl;
    if(flag & SYN || flag & FIN){
		this -> n_SEQ++;
	}
    sendto(this -> getSock(), pct, sizeof(Pacote), 0, (sockaddr *)&dest, sizeof(dest));
}

void TCP_PW::sendMsg(char const *text){
    Pacote *pct = new Pacote(this -> C_address.sin_addr, this -> getServerAddr().sin_addr, this -> C_address.sin_port,
							 this -> getServerAddr().sin_port, this -> n_ACK, this -> n_SEQ, ACK, text);
    this -> n_SEQ += (sizeof(
    sendto(this -> getSock(), pct, sizeof(Pacote), 0, (sockaddr *)this -> getServerAddrPtr(), sizeof(this -> getServerAddr()));
}

std::pair<std::pair<int, int>, std::pair<Pacote *, struct sockaddr_in> > TCP_PW::recvUDP(){
    Pacote *buff = new Pacote();
    struct sockaddr_in peer_addr;
    socklen_t peer_addr_len;
    peer_addr_len = sizeof(struct sockaddr_storage);
	struct timeval tv;
	tv.tv_sec = 0;
    tv.tv_usec = timeout * 1000000;
    if (setsockopt(this -> getSock(), SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
		perror("Error");
	}
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
				this -> n_SEQ = 0;
				this -> n_ACK = 0;
                printf("Recebido SYN\n");
                sendA("", ACK | SYN, ret.peer_addr);
                printf("Enviando ACK | SYN\n" );
                hand = 1;
                continue;
            }

            /* DISCONNECT */
            if(ret.buff -> getFlag() & FIN){
                printf("Recebido FIN\n");
                sendA("", FIN | ACK, ret.peer_addr);
                printf("Enviando ACK | FIN\n");
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
                    printf("Enviando um ACK\n");
                }
            }

            //printf("Recebido %ld bytes de %s:%s\nMensagem: %s\n\n", (long)nread, host, service, buff -> getDados());
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
