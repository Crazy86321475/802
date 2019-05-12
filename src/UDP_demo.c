#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

#include "UDP_demo.h"
#include "log_my.h"

/* 
char *str_ip = "1.2.16.255" 
AtonIp
BE_ip[0]:1 
BE_ip[1]:2 
BE_ip[2]:10 
BE_ip[3]:ff

*/
int AtonIp(char *str_ip, void *be_ip)
{
    return !inet_aton(str_ip, (struct in_addr *)be_ip);
}

int UDP_SetParam(UPUSP *p_upusp, int socket, struct sockaddr_in *server)
{

    server->sin_family = AF_INET;
    server->sin_port = htons(p_upusp->port);
    server->sin_addr.s_addr = inet_addr(p_upusp->ip);
    int opt = (server->sin_addr.s_addr == htonl(INADDR_BROADCAST)) ? 1 : 0;
    if (setsockopt(socket, SOL_SOCKET, SO_BROADCAST, &opt, sizeof(opt))) {
        LOG("ERROR: opt:%d err:%s", opt, ERRNO);
    }
    return 0;
}

int UDP_SendPkt(int socket_fd, char *pkt_buf,  unsigned int len, struct sockaddr_in *server)
{
    if (!(pkt_buf && server) || -1 == socket_fd) {
	LOG("invalid arg! socket:%d, buf:%p, server:%p",
		socket_fd, pkt_buf, server);
	return -1;
    }
    if (-1 == sendto(socket_fd, pkt_buf, len, 0, (struct sockaddr*)server, sizeof(*server))) {
        LOG("Send error! %s", ERRNO);
        return -1;
    } 
#if PC
    usleep(10); // pc send too fast
#endif
    return 0;
}


int UDP_InitSocket()
{
    int socket_fd = socket(AF_INET, SOCK_DGRAM, 0); //AF_INET -> ipv4 SOCK_DGRAM->UDP
    if (-1 == socket_fd) {
	LOG("get socket_fd failed:%s", ERRNO);
    }
    return socket_fd;
}

int UDP_DeinitSocket(int socket_fd)
{
    if (-1 == socket_fd) {
	LOG("invalid arg! socket:%d", socket_fd);
	return -1;
    }
    close(socket_fd);
    return 0;
}


