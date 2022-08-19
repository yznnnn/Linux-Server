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
    printf( "> Hello Client.\n" );
    int return_value;

    // 1. socket: 创建通信套接字
    int sockfd_comm = socket( AF_INET, SOCK_STREAM, 0 );  // IPv4  TCP
    PERROR( "socket", sockfd_comm );

    // 2. connect: 连接服务器
    sockaddr_in server_addr;
    setSockaddr_in( server_addr, "127.0.0.1", 9999 );

    return_value = connect( sockfd_comm, ( sockaddr* )&server_addr, sizeof( server_addr ) );
    PERROR( "connect", return_value );

    // 3. 通信
    char server_ip[ 16 ];
    inet_ntop( AF_INET, &server_addr.sin_addr.s_addr, server_ip, sizeof( server_ip ) );
    unsigned short server_PORT = ntohs( server_addr.sin_port );
    printf( "> A connect to [Serve@%s:%d] is accepted.\n", server_ip, server_PORT );

    while ( true ) {
        // 发送信息
        char send_buf[ 1024 ];
        printf( "[me]\t" );
        scanf( "%s", send_buf );
        return_value = write( sockfd_comm, send_buf, strlen( send_buf ) );
        PERROR( "write", return_value );

        // 接收信息
        char receive_buf[ 1024 ];
        return_value = read( sockfd_comm, receive_buf, sizeof( receive_buf ) );
        PERROR( "read", return_value );
        if ( return_value > 0 ) {
            receive_buf[ return_value ] = '\0';
            WHOAMI( "Server", server_ip, server_PORT );
            printf( "%s\n", receive_buf );
        }
        else {
            printf( "> Server closed connect.\n" );
            break;
        }
    }

    // 4. 通信结束，断开连接
    close( sockfd_comm );

    return 0;
}