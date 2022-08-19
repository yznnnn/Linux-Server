#include "Server.h"
#include <cstdio>

int main() {
    printf( "\n----------------------------------\n" );
    Server server( "127.0.0.1", 9999 );
    server.acceptConnect();
    printf( "\n----------------------------------\n" );
    return 0;
}