#include <sys/types.h>  
#include <sys/ioctl.h>  
#include <sys/socket.h>  
#include <net/if.h>  
#include <stdio.h>  
#include <stdlib.h>  
#include <unistd.h>  
#include <netdb.h>  
#include <string.h>  
#include <fcntl.h>  
#include <string.h>  
#include <errno.h>
typedef uint32_t uint32; 
#define MAX_IF 10
int main()
{
    struct ifreq ifVec[MAX_IF];

    int sock = -1;
    if ( (sock = socket( AF_INET, SOCK_DGRAM, 0 )) < 0 )
        fprintf(stderr, "Error:%d, cannot open RAM;\n");

    struct ifconf ioIfConf;
    ioIfConf.ifc_buf = (void *)ifVec;
    ioIfConf.ifc_len = sizeof(ifVec);
    printf("Len:%d\n", ioIfConf.ifc_len);     

    if (ioctl(sock, SIOCGIFCONF, &ioIfConf) < 0 )
        fprintf(stderr, "Error:%d   ioctl IFCONF\n");

    printf("Len:%d\n", ioIfConf.ifc_len);
    {
        struct ifreq *ifPt;
        struct ifreq *ifEndPt;
        ifPt = ifVec;
        ifEndPt = (void *)((char *)ifVec + ioIfConf.ifc_len);
        for (ifPt = ifVec; ifPt < ifEndPt; ifPt++)
        {
            struct ifreq ifReq;
            if ( ifPt->ifr_addr.sa_family != AF_INET ) {
                continue;
            }
            uint32 u32_addr, u32_mask;
            char ipDotBuf[16], subnetDotBuf[16], maskDotBuf[16];
            u32_addr = ((struct sockaddr_in *)&ifPt->ifr_addr)->sin_addr.s_addr;
            inet_ntop(AF_INET, &u32_addr, ipDotBuf, (socklen_t )sizeof(ipDotBuf)); 
            printf("IP Address: %s\n", ipDotBuf);
            bzero(&ifReq,sizeof(struct ifreq));  
            memcpy(ifReq.ifr_name, ifPt->ifr_name, sizeof(ifReq.ifr_name));  
            if (ioctl(sock, SIOCGIFNETMASK, &ifReq ) < 0){
                fprintf(stderr, "Error: %d, cannot get mask\n", errno);
            }
            else{
                u32_mask = ((struct sockaddr_in *)&ifReq.ifr_addr)->sin_addr.s_addr;
                inet_ntop(AF_INET, &u32_mask, maskDotBuf, (socklen_t )sizeof(maskDotBuf));
                printf("Mask: %s\n", maskDotBuf);
            }   
            bzero(&ifReq,sizeof(struct ifreq));  
            memcpy(ifReq.ifr_name, ifPt->ifr_name, sizeof(ifReq.ifr_name));  
            if (ioctl(sock, SIOCGIFMTU, &ifReq ) < 0){
                fprintf(stderr, "Error: %d, cannot get MTU\n", errno);
            }
            else{
                printf("SIOCGIFMTU:%d\n", ifReq.ifr_mtu); 
            }
        }
    }
}

