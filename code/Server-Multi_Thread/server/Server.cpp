#include "Server.h"
#include <cstdio>
#include <pthread.h>
#include <unistd.h>

static inline void PERROR( bool close_process, const char* function_name, int ret, int err_value = -1 ) {
    if ( ret == err_value ) {
        PRINT_TIME_TID();
        printf( "PERROR - " );
        perror( function_name );

        if ( close_process ) {
            PRINT_TIME_TID();
            printf( "PID-%d will exit.\n", getpid() );
            exit( -1 );
        }
        else {
            PRINT_TIME_TID();
            printf( "TID-%lu will exit.\n", pthread_self() );
            pthread_exit( nullptr );
        }
    }
}

static inline void STDERROR( const char* function_name, int ret, int success_value = 0 ) {
    if ( ret != success_value ) {
        PRINT_TIME_TID();
        printf( "STDERROR - %s: %s\n", function_name, strerror( ret ) );

        PRINT_TIME_TID();
        printf( "TID-%lu will exit.\n", pthread_self() );
        pthread_exit( nullptr );
    }
}

static inline void PRINT_TIME_TID() {
    time_t now;
    time( &now );
    struct tm* local = localtime( &now );

    printf( "\n[%d-%02d-%02d ", local->tm_year + 1900, local->tm_mon + 1, local->tm_mday );
    printf( "%02d:%02d:%02d", local->tm_hour, local->tm_min, local->tm_sec );
    printf( " TID: %lu]\n$ ", pthread_self() );
}

void* communicateReflection( void* arg ) {
    int return_value;
    // 整理接收的信息
    CommunicateInfo* com_info = ( CommunicateInfo* )arg;
    char             client_ip[ 16 ];
    unsigned short   client_PORT;
    inet_ntop( AF_INET, &com_info->client_addr.sin_addr.s_addr, client_ip, sizeof( client_ip ) );
    client_PORT = ntohs( com_info->client_addr.sin_port );
    PRINT_TIME_TID();
    printf( "I will serve for the connect to %s:%d on socket %d.\n", client_ip, client_PORT, com_info->sockfd_comm );

    char receive_buf[ 1024 ];
    while ( true ) {
        // 接收消息
        return_value = read( com_info->sockfd_comm, receive_buf, sizeof( receive_buf ) );
        PERROR( false, "read", return_value );
        if ( return_value > 0 ) {  // 有消息
            receive_buf[ return_value ] = '\0';
            PRINT_TIME_TID();
            printf( "   %s:%d ->: %s\n", client_ip, client_PORT, receive_buf );
        }
        else {  // 连接关闭
            PRINT_TIME_TID();
            printf( "The connect to %s:%d is closed.\n", client_ip, client_PORT );
            break;
        }

        // 反射消息
        return_value = write( com_info->sockfd_comm, receive_buf, return_value );
        PERROR( false, "write", return_value );
        PRINT_TIME_TID();
        printf( "-> %s:%d   : %s\n", client_ip, client_PORT, receive_buf );
    }

    PRINT_TIME_TID();
    printf( "Socket %d will be closed, TID-%lu will exit.", com_info->sockfd_comm, pthread_self() );
    close( com_info->sockfd_comm );
    pthread_exit( nullptr );
    return nullptr;
}

Server::Server( const char* IPv4_addr, uint16_t PORT ) {
    int return_value;

    PRINT_TIME_TID();
    printf( "Hello Server, you are runing on PID-%d.\n", getpid() );

    // 创建监听socket
    this->listen_sockfd = socket( AF_INET, SOCK_STREAM, 0 );
    PERROR( true, "socket", this->listen_sockfd );
    PRINT_TIME_TID();
    printf( "Listen socket fd-%d is created.\n", this->listen_sockfd );

    // 绑定监听socket
    sockaddr_in local_addr;
    local_addr.sin_family = AF_INET;
    inet_pton( local_addr.sin_family, IPv4_addr, &local_addr.sin_addr.s_addr );
    local_addr.sin_port = htons( PORT );

    return_value = bind( this->listen_sockfd, ( sockaddr* )&local_addr, sizeof( local_addr ) );
    PERROR( true, "bind", return_value );
    PRINT_TIME_TID();
    printf( "Listen socket fd-%d is bind to %s:%d\n", this->listen_sockfd, IPv4_addr, PORT );

    // 开始监听 监听socket
    return_value = listen( this->listen_sockfd, 8 );
    PERROR( true, "listen", return_value );
    PRINT_TIME_TID();
    printf( "Start listening socket fd-%d.\n", this->listen_sockfd );
}

Server::~Server() {
    close( this->listen_sockfd );
}

void Server::acceptConnect() {
    int return_value;

    sockaddr_in client_addr;
    socklen_t   client_addr_len = sizeof( client_addr );
    int         sockfd_comm     = -1;
    while ( true ) {
        sockfd_comm = accept( this->listen_sockfd, ( sockaddr* )&client_addr, &client_addr_len );
        PERROR( false, "accept", sockfd_comm );
        // 创建一个新线程负责通讯
        CommunicateInfo* com_info = new CommunicateInfo( sockfd_comm, client_addr );
        // TODO 创建线程
        pthread_t tid;
        return_value = pthread_create( &tid, nullptr, communicateReflection, ( void* )com_info );
        STDERROR( "pthread_create", return_value );
        pthread_detach( tid );

        PRINT_TIME_TID();
        printf( "A connect is established, new TID-%lu will serve for it.\n", tid );
    }
}
