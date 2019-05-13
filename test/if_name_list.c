#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>

int main()
{
	int i=0;
	int sockfd;
	struct ifconf ifc;
	unsigned char buf[512];
	struct ifreq *ifr;

	//初始化ifconf
	ifc.ifc_len = 512;
	ifc.ifc_buf = buf;

	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0))<0)
	{
		perror("socket");
		exit(1);
	}  
	ioctl(sockfd, SIOCGIFCONF, &ifc);    //获取所有接口信息

	//接下来获取逐个网卡的名称和IP地址
	ifr = (struct ifreq*)buf;  
	for(i=(ifc.ifc_len/sizeof(struct ifreq)); i>0; i--)
	{
		printf("name = [%s]\n", ifr->ifr_name);

		printf("local addr = [%s]\n",  inet_ntoa(((struct sockaddr_in*)&(ifr->ifr_addr))->sin_addr));

		ifr++;

	}
	return 0;
} 
