/*
    Construtor do pacote sem parametros
    Uso esse construtor quando vou criar um pacote vazio para receber os dados através da função recvfrom
    onde preciso passar um pacote para ele trocar pelo pacote vindo de fora
*/
Pacote::Pacote();

/*
    Construtor de verdade do pacote onde passamos o endereço de quem está enviando o pacote, o endereço do destinatário,
    a porta de origem, a porta de destino, o valor do n_ACK de quem está enviando bem como o n_SEQ, as flags e por fim
    o texto que representa a parte dos dados do pacote
*/
Pacote::Pacote(struct in_addr IP_origem, struct in_addr IP_destino, int PORT_origem, int PORT_destino,
    int n_ACK, int n_SEQ, unsigned short flag, char const  *dados);

/*
    Função que retorna a mensagem que tem dentro do pacote
*/
char * Pacote::getDados();

/*
    Função que retorna o endereço IP de origem do pacote
*/
struct in_addr Pacote::getIpOrigem();

/*
    Função que retorna o endereço IP de destino do pacote
*/
struct in_addr Pacote::getIpDest();

/*
    Função que retorna a parta de origem do pacote
*/
int Pacote::getPortOrigem();

/*
    Função que retorna a parta de destino do pacote
*/
int Pacote::getPortDest();

/*
    Função que retorna as flags (o inteiro que representa as flags) dentro do pacote
*/
unsigned short Pacote::getFlag();

/*
    Função que recupera o valor do n_ACK do pacote
*/
int Pacote::getACK();

/*
    Função que seta o valor do n_ACK do pacote
*/
void Pacote::setACK(int n_ACK);

/*
    Função que recupera o valor do n_SEQ do pacote
*/
int Pacote::getSEQ();

/*
    Função que seta o valor do n_SEQ do pacote
    Criei essa função quando fiz a parte da fragmentação do pacote porque precisava setar o n_SEQ para cada
    pacote fragmentado por partes, já que quando eu crio os pacotes o valor de n_SEQ para todos é igual e depois
    de enviar um pacote o valor do n_SEQ aumenta, então eu seto o novo n_SEQ no proximo pacote que será enviado
*/
void Pacote::setSEQ(int n_SEQ);

/*
    Construturo bem simples da classe do TCP_PW, onde passamos apenas o tipo da classe: TCP_PW_CLIENT ou TCP_PW_SERVER
*/
TCP_PW::TCP_PW(int tipo);

/*
    Função que recebe dois clocks e retorna dizendo que se estourou o tempo estipulado pelo timeout.
    Essa função é usado para basicamente dizer se "houve perda de pacote"
*/
int TCP_PW::timeHandler(clock_t s, clock_t f);

/*
    Função que faz o handshake entre o servidor e o cliente
    Basicamente o cliente chama essa função ao invocar a função connectA() que é citado logo abaixo.
    Quando essa função é chamada o cliente enviar um pacote com a flag SYN ligada para o servidor solicitando
    a conexão e então o cliente aguarda o pacote com a flag SYN e ACK para então enviar o pacote confirmando (ACK)
    a conexão. Este aguardo é explicado posteriormente com a função recvUDP()
*/
void TCP_PW::handShake();

/*
    Função que o cliente invoca para solicitar a conexão com o servidor.
    Como explicado anteriormente, esta função chama a função handShake() para estabelecer a conexão entre o cliente
    e o servidor
*/
int TCP_PW::connectA();

/*
    Função usada para finalizar a conexão entre o cliente e o servidor.
    Quem invoca esta função é o cliente. Esta função se comporta exatamente igual ao handShake() apenas
    trocando a flag SYN por FIN
*/
void TCP_PW::disconnect();

/*
    Função que inicializa as classes.
    Nesta função é onde são setados a porta que será aberta (para o servidor), a porta de destino e o ip de destino
    (para o cliente), também seta o valor do MTU (caso não tenha sido passado a flag, o MTU é setado para 1500).
    Nesta função também são criados os sockets, tanto para o cliente quanto para o servidor e, no caso do servidor,
    também é dado o "bind" na porta que será aberta para se comunicar com os clientes, enquanto que no cliente é
    aberto uma porta aleatoria válida para se comunicar com o servidor.
*/
int TCP_PW::start(int argc, char const *argv[]);

/*
    Função usado basicamente durante o handshake e o disconnect já que apenas envia flags
    Ela está aqui basicamente porque foi uma das primeiras funções que eu criei já que o cliente não mandava pacotes
    com dados para o server, então fiquei com preguiça de tirar ela e deixei ai '-'
    Os argumentos da função são as flags que serão enviadas e o endereço do destinatário, que é retornado na função
    recvfrom por referência
*/
int TCP_PW::sendA(unsigned short flag, sockaddr_in dest);

/*
    Esta é a verdadeira função de enviar coisas
    Basicamente passamos o texto que queremos enviar para o servidor e então a função faz os tratamentos devidos
    com a mensagem para então enviar para o servidor.
    Aqui não precisamos da struct com o endereço do destinatário porque a classe do cliente já guarda ela dentro da classe
    no momento que é inicializada, então apenas pegamos o endereço do servidor dentro da função pelo "this -> ".
    Aqui acontece a fragmentação, caso necessário, do texto criando vários pacotes com pedaços da mensagem que será transmi-
    tido e então guardando esses pacotes dentro de um std::vector<Pacote *> para posteriormente serem enviados em ordem
*/
void TCP_PW::sendMsg(char const *text);

/*
    Essa é a alma do treco aqui: a função que recebe os pacotins cara
    Essa deve ser uma das funções mais bizarras xD
    Basicamente criamos várias estruturas que serão usadas para guardar dados que virão do outro usuário da conexão
    no momento do envio do pacote, setamos o timeout de espera do pacote com a função setsockopt() usando a váriavel timeout
    criada no global para dizer o tempo de espera. Caso o timeout estoure, ele apenas retorna uns pair ali dizendo que
    não conseguiu ler nada e então continua esperando o outro usuário reenviar o pacote.
    Caso o pacote chegue, usamos a função getnameinfo() para pegar umas informações sobre o usuário que enviou o pacote,
    como o IP e a porta de origem do pacote (será trocado posteriomente para pegar direto do pacote recebido, já que é
    para essas coisas que estamos enviando os dados no pacote XD).
    Depois é criado uns pair loucos que serão jogados dentro de uma struct criada apenas para a recepção dos dados
*/
std::pair<std::pair<int, int>, std::pair<Pacote *, struct sockaddr_in> > TCP_PW::recvUDP();

/*
    Aqui está o loop do servidor
    Dentro desta função é colocado tudo do servidor basicamente
    Sempre que ele recebe um pacote, ele é tratado usando esta função cheia de ifs e coisas do tipo '-' Bem profissional
*/
void TCP_PW::listen();

/*
    Função que retorna o endereço do servidor
*/
struct sockaddr_in TCP_PW::getServerAddr();

/*
    Função que retorna o endereço do endereço do servidor uahsuahsuahs
*/
struct sockaddr_in * TCP_PW::getServerAddrPtr();

/*
    Função meio bosta que está aqui porque eu tinha criado sockets diferentes para o cliente e para o servidor até
    eu perceber que não precisava de dois sockets na mesma classe já que ela poderia ser apenas um servidor ou um cliente
    por vez '-' Ta ai novamente por preguiça, igual ao sendA() XD
*/
int TCP_PW::getSock();
