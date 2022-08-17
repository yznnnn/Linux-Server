#include <arpa/inet.h>
#include <asm-generic/errno-base.h>
#include <cerrno>
#include <csignal>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <ostream>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

inline void PERROR( const char* function_name, int ret, int err_value = -1 ) {
    if ( ret == err_value ) {
        perror( function_name );
        exit( -1 );
    }
}

inline void SHOWPID() {
    printf( "<PID: %d> ", getpid() );
}

inline void WHOAMI( const char* name, const char* IP, unsigned short PORT ) {
    printf( "[%s@%s:%d]\t", name, IP, PORT );
}

void setSockaddr_in( sockaddr_in& sock_addr_in, const char* IPv4, uint16_t PORT ) {  // 设置使用 IPv4 协议的 socket 地址
    sock_addr_in.sin_family = AF_INET;
    inet_pton( sock_addr_in.sin_family, IPv4, &sock_addr_in.sin_addr.s_addr );
    sock_addr_in.sin_port = htons( PORT );
}

void recycleChildProcess( int signum ) {
    while ( true ) {
        int return_value = waitpid( -1, nullptr, WNOHANG );
        if ( return_value > 0 ) {
            SHOWPID();
            printf( "Process %d has been recycled.\n", return_value );
        }
        else {
            break;
        }
    }
}

int main() {
    printf( "\n--------------------------------------------------\n" );
    SHOWPID();
    printf( "Hello Server.\n" );
    int return_value;

    // 注册信号捕捉：回收子进程
    struct sigaction act;
    act.sa_flags = 0;
    sigemptyset( &act.sa_mask );
    act.sa_handler = recycleChildProcess;
    sigaction( SIGCHLD, &act, nullptr );

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
    sockaddr_in    client_addr;
    socklen_t      client_addr_len = sizeof( client_addr );
    char           client_ip[ 16 ];
    int            sockfd_comm;
    unsigned short client_PORT;

    while ( true ) {
        // 等待连接
        sockfd_comm = accept( sockfd_listen, ( sockaddr* )&client_addr, &client_addr_len );
        if ( sockfd_comm == -1 ) {
            if ( errno == EINTR ) {
                continue;
            }
            else {
                PERROR( "accept", sockfd_comm );
            }
        }

        // 整理连接客户端信息
        inet_ntop( AF_INET, &client_addr.sin_addr.s_addr, client_ip, sizeof( client_ip ) );
        client_PORT = ntohs( client_addr.sin_port );

        // 创建子进程处理连接
        return_value = fork();
        PERROR( "fork", return_value );
        if ( return_value == 0 ) {  // 子进程，负责通信
            while ( true ) {
                char receive_buf[ 1024 ];
                return_value = read( sockfd_comm, receive_buf, sizeof( receive_buf ) );
                PERROR( "read", return_value );
                if ( return_value > 0 ) {
                    receive_buf[ return_value ] = '\0';
                    SHOWPID();
                    WHOAMI( "Client", client_ip, client_PORT );
                    printf( "%s\n", receive_buf );
                }
                else {
                    SHOWPID();
                    printf( "Client closed connect.\n" );
                    close( sockfd_comm );
                    exit( 0 );
                }

                // 发送信息
                return_value = write( sockfd_comm, receive_buf, return_value );
                PERROR( "write", return_value );
            }
        }
        else {  // 父进程，提示连接信息
            SHOWPID();
            printf( "A connect to [Client@%s:%d] is accepted, ", client_ip, client_PORT );
            printf( "Process %d will serve for it.\n", return_value );
        }
    }

    // 服务器关闭
    close( sockfd_listen );

    return 0;
}