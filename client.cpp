#include "includes/TCP_PW.hpp"

#include <iostream>

int main(int argc, char const *argv[]) {

    TCP_PW *tcp = new TCP_PW(TCP_PW_CLIENT);

    tcp -> start(argc, argv);
    tcp -> connectA();

    for(int i = 0; i < argc; i++){
        if(strcmp(argv[i], "-m") == 0){
            tcp -> sendMsg(argv[i + 1]);
            break;
        }
    }

    tcp -> disconnect();

    return 0;
}
