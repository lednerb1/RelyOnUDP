#include "includes/TCP_PW.hpp"

#include <iostream>

int main(int argc, char const *argv[]) {

    TCP_PW *tcp = new TCP_PW(TCP_PW_SERVER);

    tcp -> start(argc, argv);

    tcp -> listen();

    return 0;
}
