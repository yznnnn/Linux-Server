#ifndef YZN_SERVER_H
#define YZN_SERVER_H

#include <arpa/inet.h>
#include <bits/types/time_t.h>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

static inline void PERROR( bool close_process, const char* function_name, int ret, int err_value );
static inline void STDERROR( const char* function_name, int ret, int success_value );
static inline void PRINT_TIME_TID();

void* communicateReflection( void* arg );

struct CommunicateInfo {
    int         sockfd_comm;
    sockaddr_in client_addr;
    CommunicateInfo( int in_sockfd_comm, sockaddr_in& in_client_addr ) : sockfd_comm( in_sockfd_comm ) {
        memcpy( &this->client_addr, &in_client_addr, sizeof( in_client_addr ) );
    }
};

class Server {
private:
    int listen_sockfd = -1;

public:
    Server( const char* IPv4_addr, uint16_t PORT );
    ~Server();

    void acceptConnect();
};

#endif