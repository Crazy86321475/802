
#ifndef UDP_DEMO_H
#define UDP_DEMO_H

#include <arpa/inet.h>

#define BOARDCAST_STR_IP "255.255.255.255"

#define BE_LE_SWAP32(x) ((((unsigned int)(x) & 0xff000000) >> 24)   \
                        | (((unsigned int)(x) & 0xff0000) >> 8)    \
                        | (((unsigned int)(x) & 0xff00) << 8)    \
                        | (((unsigned int)(x) & 0xff) << 24))

typedef struct UDP_PROTOCOL_USER_SET_PARAM
{
    char ip[16];
    unsigned short port;
}UPUSP;


int AtonIp(char *str_ip, void *be_ip);

int UDP_SetParam(UPUSP *p_upusp, int socket, struct sockaddr_in *server);


int UDP_SendPkt(int socket_fd, char *send_buff, unsigned int  len, struct sockaddr_in *server);

int UDP_InitSocket();

int UDP_DeinitSocket(int socket_fd);





#endif
