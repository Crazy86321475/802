
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>

#include <stdio.h>
#include <string.h>

#include "RX802cmd_if.h"
#include "UDP_demo.h"
#include "pc2rx802_ptcl.h"
#include "netif_ctrl.h"


#include "log_my.h"

/* net */
#if PC
#define DEFAULT_LAN_IF  "enp3s0"
#else
#define DEFAULT_LAN_IF  "eth0"
#endif

#if TEST
#define TEST_HOST "222.24.86.181"
#define msleep(x)
#else
#define msleep(x) usleep((x)*1000)
#endif


#define MANY_0x33   8

#define PAR_CONFIG_DEVICE_NUM_OFFSET_SIZE_1B    0x1f7
#define PAR_CONFIG_LEFT_MARGIN_OFFSET_SIZE_2B   0x1f8
#define PAR_CONFIG_TOP_MARGIN_OFFSET_SIZE_2B    0x1fa
#define PAR_CONFIG_RIGHT_MARGIN_OFFSET_SIZE_2B  0x1fc
#define PAR_CONFIG_BOTTOM_MARGIN_OFFSET_SIZE_2B 0x1fe

static int g_devive_num = -1;
static int g_left_margin = -1;
static int g_top_margin = -1;
static int g_right_margin = -1;
static int g_bottom_margin = -1;

static UINT32 g_local_ip_he = -1;
static UINT32 g_first802_ip_he = -1;



int SendCmdRX802_many_0x33()
{
    LOG("send 10 pkt  ox33");
    UINT32 pkt_count;
    int opt = 1;
    int ret = -1;
    struct sockaddr_in server;
    int socket = UDP_InitSocket();
    if (-1 == socket) {
        LOG("ERROR!");
        return -1;
    }
    char pkt_buf[RX802_UDP_MAX_PKT_LEN];
    bzero(pkt_buf, sizeof(pkt_buf));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT_TYPE_SEND);
    server.sin_addr.s_addr = inet_addr(BOARDCAST_STR_IP);
    if (setsockopt(socket, SOL_SOCKET, SO_BROADCAST, &opt, sizeof(opt))) {
        LOG("setsockopt error! %s", ERRNO);
        goto err;
    }
    for (pkt_count = 0; pkt_count < MANY_0x33; pkt_count++) {
        //if (pkt_count % 2) {
            pkt_buf[0] = 0x33;
        //} else {
         //   pkt_buf[0] = 0x37;
        //}
	pkt_buf[1] = RX802_UDP_TRSF_STATE_FAST_PKT + 2 + pkt_count++;
	msleep(RX802_UDP_SHORT_SLEEP);
	if (UDP_SendPkt(socket, pkt_buf, sizeof(pkt_buf), &server)) {
	    goto err;
	}
    }
    msleep(RX802_UDP_SHORT_SLEEP);
    ret = 0;
    LOG("send many 0x33 over...");
err:
    UDP_DeinitSocket(socket);
    return ret;

}


int SendCmdRX802ExitPlayState(UINT32 multiple)
{
    LOG("switch RX802 state...");
    UINT32 pkt_count;
    int opt = 1;
    int ret = -1;
    struct sockaddr_in server;
    int socket = UDP_InitSocket();
    if (-1 == socket) {
        LOG("ERROR!");
        return -1;
    }
    char pkt_buf[RX802_UDP_MAX_PKT_LEN];
    bzero(pkt_buf, sizeof(pkt_buf));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT_TYPE_SEND);
    server.sin_addr.s_addr = inet_addr(BOARDCAST_STR_IP);
    if (setsockopt(socket, SOL_SOCKET, SO_BROADCAST, &opt, sizeof(opt))) {
        LOG("setsockopt error! %s", ERRNO);
        goto err;
    }
    for (pkt_count = 0; pkt_count < (RX802_UDP_TRSF_STATE_ALL_PKT * multiple); pkt_count++) {
        if (pkt_count % 2) {
            pkt_buf[0] = 0x33;
        } else {
            pkt_buf[0] = 0x37;
        }
        if (pkt_count <= RX802_UDP_TRSF_STATE_FAST_PKT) {
            pkt_buf[1] = pkt_count;
        } else {
            msleep(RX802_UDP_SHORT_SLEEP);
        }
        if (UDP_SendPkt(socket, pkt_buf, sizeof(pkt_buf), &server)) {
            goto err;
        }
    }
    msleep(RX802_UDP_SHORT_SLEEP);
    ret = 0;
    LOG("switch RX802 state over...");
err:
    UDP_DeinitSocket(socket);
    return ret;

}

int SendCmd2RX802Single(void *cmd_buf, UPUSP *upusp)
{
    struct sockaddr_in server;
    UPP upp;
    memset(&upp, 0, sizeof(upp));
    int socket = UDP_InitSocket();
    if (-1 == socket) {
        LOG("ERROR!");
        return -1;
    }
    int ret = -1;
    char pkt_buf[RX802_UDP_MAX_PKT_LEN];
    bzero(pkt_buf, sizeof(pkt_buf));

    if (RX802Cmd2PktBuff(cmd_buf, upusp, &upp, pkt_buf)) {
        LOG("RX802CmdTrans2Sendbuff failed !");
        return -1;
    }
    UDP_SetParam(&(upp.upusp), socket, &server);

    ret = UDP_SendPkt(socket, pkt_buf, RX802_UDP_MAX_PKT_LEN, &server);
    UDP_DeinitSocket(socket);
    socket = -1;
    return ret;
}

int RX802Cmd_setIp(char *set_ip)
{
    if (NULL == set_ip) {
        return -1;
    }
    LOG("setip:%s", set_ip);
    PKT_SET_IP_DATA cmd_buf;
    cmd_buf.cmd = PKT_SET_IP,
    AtonIp(set_ip, &cmd_buf.ip[0]);
    cmd_buf.mac[0] = 0x66;
    cmd_buf.mac[1] = 0x88;
    memcpy(&cmd_buf.mac[2], cmd_buf.ip, sizeof(cmd_buf.ip));
    //cmd_buf.end = 'E';
    UPUSP upusp;
    strcpy(upusp.ip, BOARDCAST_STR_IP);
    upusp.port = PORT_TYPE_SEND;
    if (SendCmd2RX802Single((void *)&cmd_buf,&upusp)){
        LOG("SendCmd2RX802Single error!");
        return -1;
    }
    msleep(1000);
    SendCmdRX802_many_0x33();
    //sleep(5);
    g_first802_ip_he = *(UINT32*)&cmd_buf.ip[0];
    g_first802_ip_he = ntohl(g_first802_ip_he);
    return 0;
}

int RX802Cmd_UpdateOnline(char *ip, char *filepath)
{
    if (!ip || !filepath) { //err
        LOG("ip is null!");
        return -1;
    }
    int ret = -1;
    UPUSP upusp;
    upusp.port = PORT_TYPE_SEND;
    strcpy(upusp.ip, ip);
    LOG("upusp.ip:%s, port:%u", upusp.ip, upusp.port);

    size_t send_size = 0;   // make sure variable occupied size 4 byte at least
    ssize_t read_ret = 0;
    unsigned char *p_addr = (unsigned char *)&send_size;

    int socket = UDP_InitSocket();
    if (-1 == socket) {
        LOG("ERROR!");
        return ret;
    }
    struct sockaddr_in server;
    UDP_SetParam(&upusp, socket, &server);
    char pkt_buf[RX802_UDP_MAX_PKT_LEN];
    bzero(pkt_buf, sizeof(pkt_buf));

    PKT_UPDATE_ONLINE_DATA *p_cmd_buf = (PKT_UPDATE_ONLINE_DATA*)pkt_buf;
    p_cmd_buf->cmd = PKT_UPDATE_ONLINE;

    int fd = open(filepath, O_RDONLY);
    if (fd < 0) {
        LOG("open update file failed:%s", ERRNO);
        goto err2;
    }
    while (1) {
        memset(p_cmd_buf->bin_data, 0xff,sizeof(p_cmd_buf->bin_data));
        read_ret = read(fd, p_cmd_buf->bin_data, UPDATE_PKT_SIZE);
        if (read_ret <= 0) {
            memset(&pkt_buf[1], 0xff, sizeof(pkt_buf)-1);
        } else {
            p_cmd_buf->addr[0] = p_addr[2];
            p_cmd_buf->addr[1] = p_addr[1];
            p_cmd_buf->addr[2] = p_addr[0];
        }
        pktAddchecksum(pkt_buf);
        if (UDP_SendPkt(socket, pkt_buf, sizeof(pkt_buf), &server)){
            LOG("Send error!");
            goto err;
        }
        if (read_ret <= 0) {
            ret = 0;
            LOG("Send update.bin Over!");
            break;
        } else {
            if (send_size % 0x10000) {
                msleep(RX802_UDP_SHORT_SLEEP);
            } else {
                msleep(RX802_UDP_LONG_SLEEP);
            }
            send_size += read_ret;
        }
    }
err:
    close(fd);
err2:
    UDP_DeinitSocket(socket);
    msleep(3000);
    return ret;
}

int RX802Cmd_ControlData(char *ip, PKT_CONTROL_DATA *p_cmd_buf)
{//unicast    use boardcast send  508 Bytes =512-4(ip) of par file's head. --4.19 by shen gong
    if (!ip || !p_cmd_buf) {
        return -1;
    }
    UPUSP upusp;
    strcpy(upusp.ip, ip);
    upusp.port = PORT_TYPE_SEND;
    if (SendCmd2RX802Single((void *)&p_cmd_buf,&upusp)){
        LOG("SendCmd2RX802Single error!");
        return -1;
    }
    msleep(5000);
    return 0;
}

int RX802_parseParFileHead(char *buf)
{
    g_devive_num = (int)buf[PAR_CONFIG_DEVICE_NUM_OFFSET_SIZE_1B];
    g_left_margin = (int)*(short*)&buf[PAR_CONFIG_LEFT_MARGIN_OFFSET_SIZE_2B];
    g_top_margin = (int)*(short*)&buf[PAR_CONFIG_TOP_MARGIN_OFFSET_SIZE_2B];
    g_right_margin = (int)*(short*)&buf[PAR_CONFIG_RIGHT_MARGIN_OFFSET_SIZE_2B];
    g_bottom_margin = (int)*(short*)&buf[PAR_CONFIG_BOTTOM_MARGIN_OFFSET_SIZE_2B];
    LOG("devices num:%d, left_margin:%d, top_margin:%d, right_margin:%d, bottom_margin:%d",
        g_devive_num, g_left_margin, g_top_margin, g_right_margin, g_bottom_margin);
    return (g_devive_num > 0) ? 0 : -1;
}

int RX802Cmd_Model_if(char *filepath)
{//unicast
    if (!filepath) { //err
        LOG("ip is null!");
        return -1;
    }
    int ret = -1;
/* deal file */
    ssize_t read_ret = 0;
    size_t read_size = 0;

    char read_buf[4 + RX802_UDP_MAX_PKT_LEN]; //4:ip
    char* const pkt_buf = &read_buf[4];
    bzero(read_buf, sizeof(read_buf));

    int fd = open(filepath, O_RDONLY);
    if (fd < 0) {
        LOG("open update file failed:%s", ERRNO);
        return -1;
    }
    int d = 0;
    int socket = UDP_InitSocket();
    if (-1 == socket) {
        LOG("ERROR!");
        goto err2;
    }
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT_TYPE_SEND);
    uint32_t ip_be;
/* START: Do par head */
    for (g_devive_num = -1; (g_devive_num == -1 || d < g_devive_num); d++) {
        memset(read_buf, 0x0, sizeof(read_buf));
        read_ret = read(fd, read_buf, PAR_FILE_MODEL_BLOCK);
        if (read_ret <= 0) {
            LOG("ERRROR!read_ret:%ld; file:%s is a empty file?", read_ret, filepath);
            goto err;
        }
    /* head: set ip according to par */
        ip_be = *(uint32_t*)read_buf;
        ip_be = BE_LE_SWAP32(ip_be);  //transform ip_le to ip_be
        if (RX802Cmd_SetIp_if(inet_ntoa(*(struct in_addr*)&ip_be))) {
            goto err;
        }
    /* head: set g_devive_num & per margins by par. */
        if (RX802_parseParFileHead(read_buf)) {
            LOG("ERRROR!can not get valid g_devive_num!");
            goto err;
        }
    /* head: Send 0x31 pkt control */
        server.sin_addr.s_addr = (in_addr_t)ip_be;
        pktAddchecksum(pkt_buf);
        if (UDP_SendPkt(socket, pkt_buf, sizeof(read_buf)-(pkt_buf-read_buf), &server)) {
            LOG("Send error! %s", ERRNO);
            goto err;
        }
        sleep(5); //send 0x31 should wait 5s
    }
/* END: do head end */

/* START: do net */
    //skip 512 * g_devive_numconfig head
    /* off_t off = */lseek(fd, PAR_FILE_MODEL_BLOCK * g_devive_num, SEEK_SET);

    //SendCmdRX802ExitPlayState(g_devive_num); //need ??


    while (1) {
        for (d = 0; d < g_devive_num; d++) {
            memset(read_buf, 0x0, sizeof(read_buf));
            read_ret = read(fd, read_buf, PAR_FILE_MODEL_BLOCK);
            if (read_ret <= 0) {
                LOG("read Model.file EOS! par size:%lu", read_size);
                break;
            } else {
                if (read_ret != PAR_FILE_MODEL_BLOCK) {
                    LOG("ERRROR!:par's size %% 512 isn't 0! read_ret:%ld", read_ret);
                    goto err;
                }
                read_size += read_ret;
            }
            ip_be = *(uint32_t*)read_buf;  //transform ip_le to ip_be
            ip_be = BE_LE_SWAP32(ip_be);
            server.sin_addr.s_addr = (in_addr_t)ip_be;

#if TEST
#else
            pktAddchecksum(pkt_buf);
#endif
            if (UDP_SendPkt(socket, pkt_buf, sizeof(read_buf)-(pkt_buf-read_buf), &server)) {
                LOG("Send error! %s", ERRNO);
                goto err;
            }
        }

        if (read_ret <= 0) {
            //go to deal send 0xff 0xff
            break;
        } else {
            if (read_size == PAR_FILE_MODEL_BLOCK * g_devive_num) {
#if TEST
#else
                msleep(RX802_UDP_LONG_SLEEP);
            } else {
                msleep(RX802_UDP_SHORT_SLEEP);
#endif
            }
        }
    }
//todo send 0xff 0xff ... * g_device_num
    memset(&pkt_buf[1], 0xff, sizeof(read_buf) - (pkt_buf - read_buf));
    ip_be = (uint32_t)server.sin_addr.s_addr;
    for (d = 0; d < g_devive_num; d++) {
        ip_be = ntohl(ip_be);
        ip_be += d;     //make sure continuously ip in par file.
        server.sin_addr.s_addr = (in_addr_t)htonl(ip_be);
        if (UDP_SendPkt(socket, pkt_buf, sizeof(read_buf) - (pkt_buf - read_buf), &server)) {
            LOG("Send error! %s", ERRNO);
            goto err;
        }
        msleep(RX802_UDP_SHORT_SLEEP);
    }
    ret = g_devive_num;
    LOG("Send Model.file Over!");
err:
    UDP_DeinitSocket(socket);
err2:
    close(fd);
    return ret;
}

int RX802Cmd_ReportState(char *ip)
{//unicast
    if (!ip) {
        return -1;
    }
    int ret = -1;
    UPUSP upusp;
    strcpy(upusp.ip, ip);
    upusp.port = PORT_TYPE_SEND;
    int socket = UDP_InitSocket();
    if (-1 == socket) {
        LOG("ERROR!");
        return ret;
    }
    struct sockaddr_in  client;
    bzero(&client, sizeof(client));
    UDP_SetParam(&upusp, socket, &client);

/* if need bind serverport to recv */
    struct sockaddr_in server;
    bzero(&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT_TYPE_RECV);
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    if(bind(socket, (struct sockaddr *)&server, sizeof(server)) == -1)
    {
        LOG("bind error!");
        goto err;
    } //ifend

    char pkt_buf[RX802_UDP_MAX_PKT_LEN];
    bzero(pkt_buf, sizeof(pkt_buf));
    PKT_REPORT_STATE_DATA *p_cmd_buf = (PKT_REPORT_STATE_DATA*)pkt_buf;
    p_cmd_buf->cmd = PKT_REPORT_STATE;
    if (UDP_SendPkt(socket, pkt_buf, sizeof(pkt_buf), &client)){
        LOG("Send error!");
        goto err;
    }
    bzero(pkt_buf, sizeof(pkt_buf));

    struct timeval tv = {10, 0};  //timeout 10s
    if (setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(tv))) {
        LOG("setsockopt error!:%s", ERRNO);
        goto err;
    }
    ssize_t num = recv(socket, pkt_buf, RX802_UDP_MAX_PKT_LEN, 0);
    LOG("num:%ld", num);
    printf("respond:%s\n", pkt_buf);
    //PKT_REPORT_STATE_DATA_RESPOND *p_report_buf = (PKT_REPORT_STATE_DATA_RESPOND*)pkt_buf; 
    // todo parse
    if(num > 0)
    {
        printf("respond:%s\n", pkt_buf);
        int i;
        for (i = 0;num > i; i++) {
            printf("%hhx ", pkt_buf[i]);
            if (0 == i % 8) {
                printf("\n");
            }
        }
        printf("\n");
    }
    ret = 0;
err:
    UDP_DeinitSocket(socket);
    return ret;
}

int RX802_ClearArpTable()
{
    UINT32 ip_addr = htonl(g_first802_ip_he);
    LOG("clear g_devive_num:%d, g_first802_ip_he:%s.", g_devive_num, inet_ntoa(*(struct in_addr*)&ip_addr));
    if (g_devive_num <= 0) {
        g_first802_ip_he = -1;
        return 0;
    }
    if (g_first802_ip_he == -1) {
        LOG("ERROR!");
        return -1;
    }
    UINT32 ip_be;
    int d;
    for (d = 0; d < g_devive_num; d++) {
        ip_be = htonl(g_first802_ip_he + d);
        arpDel(DEFAULT_LAN_IF, inet_ntoa(*(struct in_addr*)&ip_be));
    }
    return 0;
}


int RX802_BuildArpTable()
{
    //eg  arp :  192.168.1.255 -> 0x66 0x88 0xc0 0xa8 0x01 {0xip[4]}
    UINT32 ip_addr = htonl(g_first802_ip_he);
    LOG("make g_devive_num:%d, g_first802_ip_he:%s.", g_devive_num, inet_ntoa(*(struct in_addr*)&ip_addr));
    int ret = 0;
    UINT32 ip_be;
    char mac[6] = {0x66, 0x88, 0, 0, 0, 0};
    char mac_str[18] = {0};
    int d;
    for (d = 0; d < g_devive_num; d++) {
        ip_be = htonl(g_first802_ip_he + d);
        memcpy(&mac[2], &ip_be, sizeof(ip_be));
        snprintf(mac_str, sizeof(mac_str), "%02hhx:%02hhx:%02hhx:%02hhx:%hhx:%hhx",
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        ret += arpSet(DEFAULT_LAN_IF, inet_ntoa(*(struct in_addr*)&ip_be), mac_str);
    }
    return ret;
}


int RX802Cmd_SetIp_if(char *ip)
{//if ip is 192.168.1.20, so local ip is 192.168.1.2
    /* to Clear arp table */
    RX802_ClearArpTable();

    UINT32 ip_be;
    if (AtonIp(ip, &ip_be)) {
        LOG("AtonIp failed!");
        return -1;
    }

    ip_be = ntohl(ip_be);
    ip_be &= 0xffffff00;
    ip_be |= 0x5;//now ip is xxx.xxx.xxx.5
    ip_be = htonl(ip_be);
    if (SetLocalIp(DEFAULT_LAN_IF, inet_ntoa(*(struct in_addr*)&ip_be))) {
        return -1;
    }
    g_local_ip_he = ntohl(ip_be);
/* to add default route */
    system("route add default "DEFAULT_LAN_IF);
#if TEST
#else
    SendCmdRX802ExitPlayState(1);
#endif
    if (RX802Cmd_setIp(ip)) {
        LOG("failed!");
        return -1;
    }
/* to Build arp table */
    g_devive_num = 1;
    RX802_BuildArpTable();
    return 0;
}

int RX802Cmd_Update_if(char *ip, char *filepath)
{
#if TEST
#else
    SendCmdRX802ExitPlayState(1);
#endif
    return RX802Cmd_UpdateOnline(ip, filepath);
}
