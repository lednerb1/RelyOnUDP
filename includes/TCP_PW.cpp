#include "TCP_PW.hpp"

/*
    14/06 - 00:30 (Felipe Weiss)
        1. Criado um menuzinho para o cliente poder simular as etapas de envio parte a parte
        2. Começando agora a parte da numeração do SEQ e ACK (precisa ser revisado como funciona para implementar)
        3. Ajustado o inicio do cliente para setar uma porta aleatória para iniciar a conversa (feat Leonardo Valerio)
        4. Criado a flag COF (Connection Finished) para avisar o programa se o cliente está conectado ao servidor.
           Usado na função disconnect() para evitar que o cliente se desconecte do servidor se ele ainda não se
           conectou
        5. Mudado o tipo da flag para unsigned short (feat Leonardo Valerio)
        6. Adicionado loop quando o cliente enviar uma mensagem para esperar a resposta do servidor (ACK) ou para reenviar
           a mensagem caso ela tenha sido (provavelmente) perdida na rede.
        7. Aprimorado saida de dados no terminal durante os eventos/etapas do TCP. Agora tem um separador (#) para separar
           dois "eventos" e também mostra dados de quem enviou a mensagem e o tamanho do pacote recebido (sempre fixo S:
           Tem que achar um jeito sinistro de resolver isso ;-;)

    14/06 - 21:08 (Felipe Weiss)
        1. Adionado um "fragmentador" dentro da função sendMsg() (função usada pelo cliente para enviar mensagem)
        2. Ajustado o valor do MSS para ser relativo ao tamanho do pacote e do MTU
        3. Acredito que tenha feito a parte da verificação dos ACK e dos SEQ entre o servidor e o cliente
        4. Adicionado as funções de get e set do pacote para o n_SEQ e o n_ACK
        5. Aprimorado um pouco mais as saídas dos dados no terminal. Agora temos flechas mostrando se o dado
           em questão é de entrada ou de saída (<-- Dado de entrada (vindo de fora) | --> Dado de saída (oriundo do
           programa "principal") | - Apenas uma informação sobre algum evento ocorrido)

    14/06 - 22:01 (Felipe Weiss)
        1. Criado o arquivo comentarios.cpp, que é basicamente uma descriçaõ das funções deste arquivo
*/

int MTU = 1500;
int MSS = 2 * MTU - sizeof(Pacote);
double timeout = 0.1;

Pacote::Pacote(){
    struct in_addr a;
    Pacote(a, a, 8080, 8080, 0, 0, 0, "");
}

Pacote::Pacote(struct in_addr IP_origem, struct in_addr IP_destino, int PORT_origem, int PORT_destino,
    int n_ACK, int n_SEQ, unsigned short flag, char const  *dados){
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
unsigned short Pacote::getFlag(){
    return this -> flag;
}
int Pacote::getACK(){
    return this -> n_ACK;
}
void Pacote::setACK(int n_ACK){
    this -> n_ACK = n_ACK;
}
int Pacote::getSEQ(){
    return this -> n_SEQ;
}
void Pacote::setSEQ(int n_SEQ){
    this -> n_SEQ = n_SEQ;
}

TCP_PW::TCP_PW(int tipo){
    this -> tipo = tipo;
    this -> handC = 0;
}

int TCP_PW::timeHandler(clock_t s, clock_t f){
	double t = (f - s) * 1000.0 / CLOCKS_PER_SEC;
	if(t > timeout){
		printf("----Provavel perda de pacote----\n");
		return 1;
	}
	return 0;
}

/* Client */
void TCP_PW::handShake(){
	printf("--> Enviando SYN\n");
    sendA(SYN, this -> getServerAddr());
    this -> n_SEQ++;
    clock_t start = clock();
    while(1){
        InfoRetRecv ret = InfoRetRecv(recvUDP());
        if(ret.s == 0){
            if(ret.buff -> getFlag() == ACK | SYN && ret.buff -> getACK() == this -> n_SEQ){
                printf("<-- Recebido ACK | SYN\n");
                sendA(ACK, ret.peer_addr);
                printf("--> Enviando ACK\n");
                break;
            }
        }
        if(timeHandler(start, clock())){
			printf("--> Enviando SYN\n");
			sendA(SYN, this -> getServerAddr());
			start = clock();
		}
    }
    printf("- Conectado ao servidor!\n");
    this -> handC = 1;
}

int TCP_PW::connectA(){
    if(this -> handC == 1){
        printf("- Ja conectado ao servidor!!\n");
        return -1;
    }
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

void TCP_PW::disconnect(){
    if(!this -> handC){
        printf("- Ainda nao conectado ao servidor!\n");
        return;
    }
    printf("--> Enviando FIN\n");
    sendA(FIN, this -> getServerAddr());
    clock_t start = clock();
    while(1){
        InfoRetRecv ret = InfoRetRecv(recvUDP());
        if(ret.s == 0){
            if(ret.buff -> getFlag() == ACK | FIN){
                printf("<-- Recebido ACK | FIN\n");
                sendA(ACK, ret.peer_addr);
                printf("--> Enviando ACK\n");
                break;
            }
        }
        if(timeHandler(start, clock())){
            printf("--> Enviando FIN\n");
			sendA(FIN, this -> getServerAddr());
			start = clock();
		}
    }
    printf("- Desconectado do servidor!\n");
    this -> handC = 0;
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
        int ret = -1;
        srand(time(NULL));
        while(ret < 0){
            this -> C_address.sin_port = htons(rand() % 8000 + 1500);
            ret = bind(this -> sock, (struct sockaddr *)&(this -> C_address), sizeof(this -> C_address));
        }

        // if(getsockname(this -> sock, (struct sockaddr *)&(this -> C_address), this -> sock)){
        //     perror("getsockname failed");
        //     return -1;
        // }

        this -> serv_addr.sin_family = AF_INET;
        this -> serv_addr.sin_addr.s_addr = inet_addr(this -> IP);
        this -> serv_addr.sin_port = htons(this -> PORT);
    }
}

int TCP_PW::sendA(unsigned short flag, sockaddr_in dest){
    Pacote *pct;
    if(this -> tipo == TCP_PW_CLIENT){
        pct = new Pacote(this -> C_address.sin_addr, dest.sin_addr, this -> C_address.sin_port, dest.sin_port, this -> n_ACK, this -> n_SEQ, flag, "");
    } else {
        pct = new Pacote(this -> S_address.sin_addr, dest.sin_addr, this -> S_address.sin_port, dest.sin_port, this -> n_ACK, this -> n_SEQ, flag, "");
    }

    sendto(this -> getSock(), pct, sizeof(Pacote), 0, (sockaddr *)&dest, sizeof(dest));
}

void TCP_PW::sendMsg(char const *text){
    if(this -> handC){
        std::vector<Pacote *> pacotes;
        std::string txt = "";
        for(int i = 0; i < strlen(text); i++){
            if(i && i % MSS == 0){
                pacotes.push_back(new Pacote(this -> C_address.sin_addr, this -> getServerAddr().sin_addr, this -> C_address.sin_port,
            							     this -> getServerAddr().sin_port, this -> n_ACK, this -> n_SEQ, ACK, txt.c_str()));
                txt = "";
            }
            txt += text[i];
        }
        pacotes.push_back(new Pacote(this -> C_address.sin_addr, this -> getServerAddr().sin_addr, this -> C_address.sin_port,
                                     this -> getServerAddr().sin_port, this -> n_ACK, this -> n_SEQ, ACK, txt.c_str()));
        for(Pacote *p : pacotes){
            p -> setSEQ(this -> n_SEQ);
            //Pacote *pct = new Pacote(this -> C_address.sin_addr, this -> getServerAddr().sin_addr, this -> C_address.sin_port,
        	//						 this -> getServerAddr().sin_port, this -> n_ACK, this -> n_SEQ, ACK, text);
            printf("--> Enviando mensagem!\n");
            sendto(this -> getSock(), p, sizeof(Pacote), 0, (sockaddr *)this -> getServerAddrPtr(), sizeof(this -> getServerAddr()));
            clock_t start = clock();
            while(1){
                InfoRetRecv ret = InfoRetRecv(recvUDP());
                if(ret.s == 0){
                    if(ret.buff -> getFlag() & ACK && ret.buff -> getACK() == this -> n_SEQ){
                        printf("<-- Recebido ACK\n");
                        break;
                    }
                }
                if(timeHandler(start, clock())){
                    printf("--> Enviando mensagem!\n");
        			sendto(this -> getSock(), p, sizeof(Pacote), 0, (sockaddr *)this -> getServerAddrPtr(), sizeof(this -> getServerAddr()));
        			start = clock();
                }
            }
            this -> n_SEQ++;
        }
    } else {
        printf("- Ainda nao conectado ao servidor!\n");
    }
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
    printf("#########################################################################\n");
    printf("<-- Recebido %ld bytes de %s:%s | n_SEQ: %d   n_ACK: %d\n", (long)nread, host, service, buff -> getSEQ(), buff -> getACK());
    return {{s, nread}, {buff, peer_addr}};
}

void TCP_PW::listen(){
    int hand = 0, disc = 0;
    std::string msg = "";
    while(1){
        InfoRetRecv ret = InfoRetRecv(recvUDP());
        if(ret.s == 0){
            /* HANDSHAKE */
            if(ret.buff -> getFlag() & SYN){
				this -> n_SEQ = 0;
				this -> n_ACK = 1;
                printf("<-- Recebido SYN\n");
                printf("--> Enviando ACK | SYN\n");
                sendA(ACK | SYN, ret.peer_addr);
                hand = 1;
                continue;
            }

            /* DISCONNECT */
            if(ret.buff -> getFlag() & FIN){
                if(this -> handC){
                    printf("<-- Recebido FIN\n");
                    sendA(FIN | ACK, ret.peer_addr);
                    printf("--> Enviando ACK | FIN\n");
                    disc = 1;
                }
                continue;
            }

            /* Recebeu um ACK */
            if(ret.buff -> getFlag() & ACK){
                printf("<-- Recebido ACK\n");
                /* Se for um ACK para confirmar a conexão */
                if(hand){
                    printf("- Conexao estabelecida\n");
                    this -> handC = 1;
                    hand = 0;
                    msg = "";
                } else if(disc){ /* Se for um ACK para confirmar o encerramento da conexão */
                    printf("- Conexao encerrada\n");
                    disc = 0;
                    this -> handC = 0;
                } else {
                    if(ret.buff -> getSEQ() == this -> n_ACK){
                        printf("<-- Mensagem recebida: %s\n", ret.buff -> getDados());
                        msg += ret.buff -> getDados();
                        printf("<-- Mensagem ate agora: %s", msg.c_str());
                        sendA(ACK, ret.peer_addr);
                        printf("\n--> Enviando um ACK\n");
                        this -> n_ACK++;
                    }
                }
            }
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
