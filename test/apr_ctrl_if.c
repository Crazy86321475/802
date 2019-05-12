#include <stdio.h> 
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if_arp.h>
#include <string.h>

#include <ifaddrs.h>

#include <error.h>

/*******************************************************************************
 * 函 数 名    :    arpDel
 * 函数功能    :    LINUX删除ARP表项
 * 输入参数    :    
 char *ifname     :   网络接口名，如eth0
 char *ipStr      :   需要删除的IP，点分十进制串
 * 输出参数    :
 * 返 回 值    :
0        :    成功
-1        :    失败
 *******************************************************************************/
int arpDel(char *ifname, char *ipStr)
{
    if(ifname == NULL || ipStr == NULL)
    {
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
    if(sock_fd < 0)
    {
        printf("get socket error.\n");
        return -1;
    }

    ret = ioctl(sock_fd, SIOCDARP, &req);
    if(ret < 0)
    {
        perror("ioctl error.\n");
        close(sock_fd);
        return -1;
    }

    close(sock_fd);
    return 0;
}


/*******************************************************************************
 * 函 数 名    :    arpGet
 * 函数功能    :    LINUX获取ARP表项
 * 输入参数    :    
 char *ifname     :   网络接口名，如eth0
 char *ipStr      :   需要删除的IP，点分十进制串
 * 输出参数    :
 * 返 回 值    :
0        :    成功
-1        :    失败
 *******************************************************************************/
int arpGet(char *ifname, char *ipStr)
{
    if(ifname == NULL || ipStr == NULL)
    {
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
    if(sock_fd < 0)
    {
        printf("get socket error.\n");
        return -1;
    }

    ret = ioctl(sock_fd, SIOCGARP, &req);
    if(ret < 0)
    {
        perror("ioctl error.\n");
        close(sock_fd);
        return -1;
    }

    unsigned char *hw = (unsigned char *)req.arp_ha.sa_data;
    printf("%#x-%#x-%#x-%#x-%#x-%#x\n", hw[0], hw[1], hw[2], hw[3], hw[4], hw[5]);
    printf("%#x\n", req.arp_flags);
    close(sock_fd);
    return 0;
}





int getHwAddr(char *buff, char *mac)
{
    if( buff == NULL || mac == NULL )
    {
        return -1;
    }

    int i = 0;
    unsigned int p[6];

    if(sscanf(mac, "%x:%x:%x:%x:%x:%x", &p[0], &p[1], &p[2], &p[3], &p[4], &p[5]) < 6)
    {
        return -1;
    }

    for(i = 0; i < 6; i ++)
    {
        buff[i] = p[i];
    }

    return 0;
}



/*******************************************************************************
 * 函 数 名    :    arpSet
 * 函数功能    :    LINUX增加ARP表项
 * 输入参数    :    
 char *ifname     :   网络接口名，如eth0
 char *ipStr      :   IP，点分十进制串
 char *mac        :   MAC地址，如00:a1:b2:c3:d4:e5
 * 输出参数    :
 * 返 回 值    :
0        :    成功
-1        :    失败
 *******************************************************************************/
int arpSet(char *ifname, char *ipStr, char *mac)
{
    if(ifname == NULL || ipStr == NULL || mac == NULL)
    {
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
    req.arp_flags = ATF_PERM | ATF_COM;

    if(getHwAddr((char *)req.arp_ha.sa_data, mac) < 0)
    {
        printf("get mac error.\n");
        return -1;
    }

    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock_fd < 0)
    {
        printf("get socket error.\n");
        return -1;
    }

    ret = ioctl(sock_fd, SIOCSARP, &req);
    if(ret < 0)
    {
        perror("ioctl error.\n");
        close(sock_fd);
        return -1;
    }

    close(sock_fd);
    return 0;
}


int getSubnetMask()
{
    struct sockaddr_in *sin = NULL;
    struct ifaddrs *ifa = NULL, *ifList;

    if (getifaddrs(&ifList) < 0)
    {
        return -1;
    }

    for (ifa = ifList; ifa != NULL; ifa = ifa->ifa_next)
    {
        if(ifa->ifa_addr->sa_family == AF_INET)
        {
            printf("n>>> interfaceName: %s\n", ifa->ifa_name);

            sin = (struct sockaddr_in *)ifa->ifa_addr;
            printf(">>> ipAddress: %s\n", inet_ntoa(sin->sin_addr));

            sin = (struct sockaddr_in *)ifa->ifa_dstaddr;
            printf(">>> broadcast: %s\n", inet_ntoa(sin->sin_addr));

            sin = (struct sockaddr_in *)ifa->ifa_netmask;
            printf(">>> subnetMask: %s\n", inet_ntoa(sin->sin_addr));
        }
    }

    freeifaddrs(ifList);

    return 0;
}

int main(int argc, char *argv[])
{
    printf("---------------------------------------\n");
    arpSet("enp3s0", "5.5.5.6", "00:a3:b4:c5:d6:e5");
    //printf("---------------------------------------\n");
    arpDel("enp3s0", "5.5.5.5");
    getSubnetMask();
    //printf("---------------------------------------\n");
    //printf("retvalue=%d\n", arpDel("enp3s0", "5.5.5.5"));
    //printf("---------------------------------------\n");
    //arpGet("enp3s0", "5.5.5.5");

    return 0;
}
