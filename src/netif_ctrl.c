#include <stdio.h> 
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if_arp.h>
#include <net/if.h>
#include <ifaddrs.h>
//#include <netdb.h> 


#include "log_my.h"
//precondition: must need root

int arpDel(char *ifname, char *ipStr)
{
    if (ifname == NULL || ipStr == NULL) {
        printf("para is null.\n");
        return -1;
    }

    struct arpreq req;
    struct sockaddr_in *sin;
    int ret = 0;
    int sock_fd = 0;

    memset(&req, 0, sizeof(struct arpreq));
    sin = (struct sockaddr_in *)&req.arp_pa;
    sin->sin_family = AF_INET;
    sin->sin_addr.s_addr = inet_addr(ipStr);
    //arp_dev长度为[16]，注意越界
    strncpy(req.arp_dev, ifname, 15);

    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd < 0) {
        LOG("Get sock error.:%s", ERRNO);
        return -1;
    }

    ret = ioctl(sock_fd, SIOCDARP, &req);
    if (ret < 0) {
        LOG("ioctl error.:%s", ERRNO);
        close(sock_fd);
        return -1;
    }

    close(sock_fd);
    return 0;
}

int arpGet(char *ifname, char *ipStr)
{
    if (ifname == NULL || ipStr == NULL) {
        printf("para is null.\n");
        return -1;
    }

    struct arpreq req;
    struct sockaddr_in *sin;
    int ret = 0;
    int sock_fd = 0;

    memset(&req, 0, sizeof(struct arpreq));

    sin = (struct sockaddr_in *)&req.arp_pa;
    sin->sin_family = AF_INET;
    sin->sin_addr.s_addr = inet_addr(ipStr);

    //arp_dev长度为[16]，注意越界
    strncpy(req.arp_dev, ifname, 15);

    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd < 0) {
        LOG("Get sock error.:%s", ERRNO);
        return -1;
    }

    ret = ioctl(sock_fd, SIOCGARP, &req);
    if (ret < 0) {
        LOG("ioctl error.:%s", ERRNO);
        close(sock_fd);
        return -1;
    }

    unsigned char *hw = (unsigned char *)req.arp_ha.sa_data;
    LOG("%#x-%#x-%#x-%#x-%#x-%#x", hw[0], hw[1], hw[2], hw[3], hw[4], hw[5]);
    LOG("%#x\n", req.arp_flags);
    close(sock_fd);
    return 0;
}

int getHwAddr(char *buff, char *mac)
{
    if (buff == NULL || mac == NULL) {
        return -1;
    }

    int i = 0;
    unsigned int p[6];

    if (sscanf(mac, "%x:%x:%x:%x:%x:%x", &p[0], &p[1], &p[2], &p[3], &p[4], &p[5]) < 6) {
        return -1;
    }

    for (i = 0; i < 6; i++) {
        buff[i] = p[i];
    }

    return 0;
}

int arpSet(char *ifname, char *ipStr, char *mac)
{
    if (ifname == NULL || ipStr == NULL || mac == NULL) {
        LOG("para is null.");
        return -1;
    }

    struct arpreq req;
    struct sockaddr_in *sin;
    int ret = 0;
    int sock_fd = 0;

    memset(&req, 0, sizeof(struct arpreq));
    sin = (struct sockaddr_in *)&req.arp_pa;
    sin->sin_family = AF_INET;
    sin->sin_addr.s_addr = inet_addr(ipStr);
    //arp_dev长度为[16]，注意越界
    strncpy(req.arp_dev, ifname, 15);
    req.arp_flags = ATF_PERM | ATF_COM;

    if (getHwAddr((char *)req.arp_ha.sa_data, mac) < 0) {
        LOG("get mac error.");
        return -1;
    }

    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd < 0) {
        LOG("Get sock error.:%s", ERRNO);
        return -1;
    }

    ret = ioctl(sock_fd, SIOCSARP, &req);
    if (ret < 0) {
        LOG("ioctl error.:%s", ERRNO);
        close(sock_fd);
        return -1;
    }

    close(sock_fd);
    return 0;
}


int ShowNetConfig()
{
    struct sockaddr_in *sin = NULL;
    struct ifaddrs *ifa = NULL, *ifList;

    if (getifaddrs(&ifList) < 0) {
        return -1;
    }

    for (ifa = ifList; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr->sa_family == AF_INET) {
            LOG("n>>> interfaceName: %s", ifa->ifa_name);

            sin = (struct sockaddr_in *)ifa->ifa_addr;
            LOG(">>> ipAddress: %s", inet_ntoa(sin->sin_addr));

            sin = (struct sockaddr_in *)ifa->ifa_dstaddr;
            LOG(">>> broadcast: %s", inet_ntoa(sin->sin_addr));

            sin = (struct sockaddr_in *)ifa->ifa_netmask;
            LOG(">>> subnetMask: %s", inet_ntoa(sin->sin_addr));
        }
    }

    freeifaddrs(ifList);

    return 0;
}

int CheckLocalNetIfNameExist(char *ifName)
{
    struct ifaddrs *ifa = NULL, *ifList;
    if (getifaddrs(&ifList) < 0) {
        LOG("Make if list failed! %s", ERRNO);
        return -1;
    }

    for (ifa = ifList; ifa != NULL; ifa = ifa->ifa_next) {
        if(ifa->ifa_addr->sa_family == AF_INET) {
            LOG("if:%s", ifa->ifa_name);
            if (0 == strcmp(ifName, ifa->ifa_name)) {
                LOG("Exist, %s in devices.", ifName);
                return 1;
            }
        }
    }
    freeifaddrs(ifList);
    return 0;
}

int SetLocalIp(char *ifName, const char *ipaddr)
{
    if (CheckLocalNetIfNameExist(ifName) <= 0) {
        LOG("Check failed!if:%s", ifName);
        return -1;
    }
    struct sockaddr_in sin_set_ip;//网络设置结构
    struct ifreq ifr_set_ip;//单个网络接口设置结构
    bzero(&ifr_set_ip, sizeof(ifr_set_ip));
    if (ipaddr == NULL) {
        return -1;
    }
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        LOG("Get sock error.:%s", ERRNO);
        return -1;
    }

    memset(&sin_set_ip, 0, sizeof(sin_set_ip));
    strncpy(ifr_set_ip.ifr_name, ifName, sizeof(ifr_set_ip.ifr_name)-1);
    sin_set_ip.sin_family = AF_INET;
    sin_set_ip.sin_addr.s_addr = inet_addr(ipaddr);
    memcpy( &ifr_set_ip.ifr_addr, &sin_set_ip, sizeof(sin_set_ip));
    /* SIOCGIFFLAGS, SIOCSIFFLAGS
        读取 或 设置 设备 */
    if (ioctl(sock, SIOCSIFADDR, &ifr_set_ip) < 0) {
        LOG("ioctl error.:%s", ERRNO);
        close(sock);
        return -1;
    }
    //设置激活标志
    ifr_set_ip.ifr_flags |= IFF_UP |IFF_RUNNING;

    //get the status of the device
    if (ioctl(sock, SIOCSIFFLAGS, &ifr_set_ip) < 0) {
        LOG("ioctl error.:%s", ERRNO);
        close(sock);
        return -1;
    }

    close(sock);
    return 0;
}
