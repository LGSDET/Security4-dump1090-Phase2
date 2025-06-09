// SbsServer_Linux.c - Linux-compatible version of the original Windows-based SbsServer.cpp

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>

#if 1
#define LG_SECURITY_ENHANCEMENT
#define LG_SECURITY_ENHANCEMENT_SQLOG
#include "sqlog.h"
#define LG_SECURITY_USE_EXT_SBS_THREAD
#endif

#if 0
#define DEFAULT_SERVER_NAME "data.adsbhub.org"
#else
#define DEFAULT_SERVER_NAME "128.237.96.41"
#endif
#define DEFAULT_PORT_STR "5002"
#define DEFAULT_PORT 5002
#define MAX_CLIENTS 100
#define DEFAULT_BUFLEN 4096

#define CBSIZE 2048

typedef struct cbuf
{
    char buf[CBSIZE];
    int fd;
    unsigned int rpos, wpos;
} cbuf_t;

cbuf_t stringbuffer;

int RemoteSocket = -1;
#ifdef DO_SERVER_ROLE
int ServerSocket = -1;
int ClientSockets[MAX_CLIENTS] = {};
#endif

int RunServer(void);
int ReadLine(char *dst, unsigned int size);
#ifdef DO_SERVER_ROLE
int ServerSetup(void);
#endif

#ifdef LG_SECURITY_ENHANCEMENT
// This is from dump1090.c
void modesSendAllClients(int service, void *msg, int len);
int dump1090_get_tlsbsos(void);
#endif

int ReadLine(char *dst, unsigned int size)
{
    unsigned int i = 0;
    int n;
    while (i < size)
    {
        if (stringbuffer.rpos == stringbuffer.wpos)
        {
            int wpos = stringbuffer.wpos % CBSIZE;
            n = recv(RemoteSocket, stringbuffer.buf + wpos, CBSIZE - wpos, 0);

            if (n < 0)
                return -1;
            else if (n == 0)
                return 0;
            stringbuffer.wpos += n;
        }
        dst[i++] = stringbuffer.buf[stringbuffer.rpos++ % CBSIZE];
        if (dst[i - 1] == '\n')
            break;
    }
    if (i == size)
    {
        fprintf(stderr, "line too large: %d %d\n", i, size);
        return -1;
    }
    dst[i] = 0;
    return i;
}

#ifdef DO_SERVER_ROLE
int ServerSetup(void)
{
    int opt = 1;
    struct sockaddr_in server;

    for (int i = 0; i < MAX_CLIENTS; i++)
        ClientSockets[i] = -1;

    ServerSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (ServerSocket < 0)
    {
        perror("Could not create socket");
        return 2;
    }

    if (setsockopt(ServerSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("setsockopt");
        return 3;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(DEFAULT_PORT);

    if (bind(ServerSocket, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("bind failed");
        return 4;
    }

    listen(ServerSocket, MAX_CLIENTS);
    printf("Server is waiting for incoming connections...\n");

    return 0;
}
#endif

#ifdef LG_SECURITY_ENHANCEMENT
void *SbsNetworkReaderThread(void *pParam)
#else
int main(int argc, char **argv)
#endif
{
    struct addrinfo hints, *result, *ptr;
    int iResult, ErrorCount = 0;

#ifdef LG_SECURITY_ENHANCEMENT
    (void)pParam;
#endif

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

#ifdef DO_SERVER_ROLE
    if ((iResult = ServerSetup()) != 0)
    {
        return 1;
    }
#endif

    while (1)
    {
        iResult = getaddrinfo(DEFAULT_SERVER_NAME, DEFAULT_PORT_STR, &hints, &result);
        if (iResult != 0)
        {
            fprintf(stderr, "getaddrinfo failed: %s\n", gai_strerror(iResult));
            ErrorCount++;
            sleep(2);
            if (ErrorCount < 10)
                continue;
#ifdef LG_SECURITY_ENHANCEMENT
            SqLog_E("Failure in %s while getaddrinfo(%s:%s) with %s\n",
                    __func__,
                    DEFAULT_SERVER_NAME,
                    DEFAULT_SERVER_NAME,
                    gai_strerror(iResult));
            return (void *)1;
#else
            return 1;
#endif
        }

        for (ptr = result; ptr != NULL; ptr = ptr->ai_next)
        {
#if 1 // print remote ip/port
            char ipstr[INET6_ADDRSTRLEN];
            void *addr;
            uint16_t port;

            // Get pointer to address (IPv4 or IPv6)
            if (ptr->ai_family == AF_INET)
            {
                struct sockaddr_in *ipv4 = (struct sockaddr_in *)ptr->ai_addr;
                addr = &(ipv4->sin_addr);
                port = ntohs(ipv4->sin_port);
            }
            else
            { // AF_INET6
                struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)ptr->ai_addr;
                addr = &(ipv6->sin6_addr);
                port = ntohs(ipv6->sin6_port);
            }

            // Convert IP to string
            inet_ntop(ptr->ai_family, addr, ipstr, sizeof(ipstr));
            printf("Trying to connect to %s:%d\n", ipstr, port);
#endif
            RemoteSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
            if (RemoteSocket < 0)
                continue;

            if (connect(RemoteSocket, ptr->ai_addr, ptr->ai_addrlen) == 0)
            {
#ifdef LG_SECURITY_ENHANCEMENT_SQLOG
                SqLog_I("Remote SBS Server %s:%d connected\n", ipstr, port);
#endif
                break;
            }

            close(RemoteSocket);
            RemoteSocket = -1;
            sleep(2);
        }
        freeaddrinfo(result);

        if (RemoteSocket != -1)
        {
#if 0 // This appeared to make server close the port
            shutdown(RemoteSocket, SHUT_WR);
#endif
            memset(&stringbuffer, 0, sizeof(stringbuffer));
            while (1)
            {
                if (RunServer() > 0)
                {
                    close(RemoteSocket);
                    RemoteSocket = -1;
                    printf("reconnecting\n");
                    break;
                }
            }
        }
    }

#ifdef LG_SECURITY_ENHANCEMENT
    return NULL;
#else
    return 0;
#endif
}

int RunServer(void)
{
    fd_set readfds;
    char message[1024];
    int messagesize;
#ifdef DO_SERVER_ROLE
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);
#endif

    FD_ZERO(&readfds);
    FD_SET(RemoteSocket, &readfds);

#ifdef DO_SERVER_ROLE
    FD_SET(ServerSocket, &readfds);
    int maxfd = ServerSocket > RemoteSocket ? ServerSocket : RemoteSocket;
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (ClientSockets[i] != -1)
        {
            FD_SET(ClientSockets[i], &readfds);
            if (ClientSockets[i] > maxfd)
                maxfd = ClientSockets[i];
        }
    }
#else
    int maxfd = RemoteSocket;
#endif

    if (select(maxfd + 1, &readfds, NULL, NULL, NULL) < 0)
    {
        perror("select error");
        return 4;
    }

    if (FD_ISSET(RemoteSocket, &readfds))
    {
        messagesize = ReadLine(message, sizeof(message));

        if (messagesize <= 0)
        {
            printf("Host disconnected\n");
            return 5;
        }
#ifdef DO_SERVER_ROLE
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (ClientSockets[i] != -1)
            {
                if (send(ClientSockets[i], message, messagesize, 0) < 0)
                {
                    close(ClientSockets[i]);
                    ClientSockets[i] = -1;
                }
            }
        }
#else
        do
        {
            static unsigned int i = 0;
            if (i == 0)
            {
#ifdef LG_SECURITY_ENHANCEMENT
                modesSendAllClients(dump1090_get_tlsbsos(), message, messagesize);
                // SqLog_I("SBS Rcvd[%d], %s\n", messagesize, message);
#else
                // printf("SBS Rcvd[%d] %s", messagesize, message);
#endif
            }
        } while (0);
#endif
    }

#ifdef DO_SERVER_ROLE
    if (FD_ISSET(ServerSocket, &readfds))
    {
        int new_socket = accept(ServerSocket, (struct sockaddr *)&address, &addrlen);
        if (new_socket < 0)
        {
            perror("accept error");
            return 6;
        }
        printf("New connection, socket fd is %d, IP is: %s, port: %d\n",
               new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (ClientSockets[i] == -1)
            {
                ClientSockets[i] = new_socket;
                break;
            }
        }
    }

    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (ClientSockets[i] != -1 && FD_ISSET(ClientSockets[i], &readfds))
        {
            char client_message[DEFAULT_BUFLEN];
            int len = recv(ClientSockets[i], client_message, sizeof(client_message) - 1, 0);
            if (len <= 0)
            {
                close(ClientSockets[i]);
                ClientSockets[i] = -1;
            }
            else
            {
                client_message[len] = '\0';
            }
        }
    }
#endif

    return 0;
}
