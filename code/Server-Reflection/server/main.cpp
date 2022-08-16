#include <arpa/inet.h>
#include <cstdio>
#include <cstring>
#include <ostream>
#include <unistd.h>

inline void PERROR( const char* function_name, int ret, int err_value = -1 ) {
    if ( ret == err_value ) {
        perror( function_name );
        exit( -1 );
    }
}

inline void WHOAMI( const char* name, const char* IP, unsigned short PORT ) {
    printf( "[%s@%s:%d]\t", name, IP, PORT );
}

void setSockaddr_in( sockaddr_in& sock_addr_in, const char* IPv4, uint16_t PORT ) {  // 设置使用 IPv4 协议的 socket 地址
    sock_addr_in.sin_family = AF_INET;
    inet_pton( sock_addr_in.sin_family, IPv4, &sock_addr_in.sin_addr.s_addr );
    sock_addr_in.sin_port = htons( PORT );
}

int main() {
    printf( "\n--------------------------------------------------\n" );
    printf( "> Hello Server.\n" );
    int return_value;

    // 1. socket: 创建监听套接字
    int sockfd_listen = socket( AF_INET, SOCK_STREAM, 0 );  // IPv4  TCP
    PERROR( "socket", sockfd_listen );

    // 2. bind: 绑定进程地址（IP地址+端口号）
    sockaddr_in local_addr;
    setSockaddr_in( local_addr, "127.0.0.1", 9999 );

    return_value = bind( sockfd_listen, ( sockaddr* )&local_addr, sizeof( local_addr ) );
    PERROR( "bind", return_value );

    // 3. listen: 监听
    return_value = listen( sockfd_listen, 8 );
    PERROR( "listen", return_value );

    // 4. accept: 接受连接
    sockaddr_in client_addr;
    socklen_t   client_addr_len = sizeof( client_addr );

    int sockfd_comm = accept( sockfd_listen, ( sockaddr* )&client_addr, &client_addr_len );
    PERROR( "accept", sockfd_comm );

    // 5. 通信
    char client_ip[ 16 ];
    inet_ntop( AF_INET, &client_addr.sin_addr.s_addr, client_ip, sizeof( client_ip ) );
    unsigned short client_PORT = ntohs( client_addr.sin_port );
    printf( "> A connect is accepted. " );
    printf( "Client IP: %s, Clinet process PORT: %d\n", client_ip, client_PORT );

    // 接收信息并发送回去
    while ( true ) {
        char receive_buf[ 1024 ];
        return_value = read( sockfd_comm, receive_buf, sizeof( receive_buf ) );
        PERROR( "read", return_value );
        if ( return_value > 0 ) {
            receive_buf[ return_value ] = '\0';
            WHOAMI( "Client", client_ip, client_PORT );
            printf( "%s\n", receive_buf );
        }
        else {
            printf( "> Client closed connect.\n" );
            break;
        }

        // 发送信息
        return_value = write( sockfd_comm, receive_buf, return_value );
        PERROR( "write", return_value );
    }

    // 6. 通信结束，断开连接
    close( sockfd_listen );
    close( sockfd_comm );

    return 0;
}