#include "includes/TCP_PW.hpp"

#include <iostream>

int main(int argc, char const *argv[]) {

    TCP_PW *tcp = new TCP_PW(TCP_PW_CLIENT);

    tcp -> start(argc, argv);
    tcp -> connectA();

    tcp -> disconnect();

    return 0;
}
