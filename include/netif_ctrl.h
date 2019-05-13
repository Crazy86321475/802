#ifndef NETIF_CTRL_H
#define NETIF_CTRL_H


int arpDel(char *ifname, char *ipStr);
int arpGet(char *ifname, char *ipStr);
int arpSet(char *ifname, char *ipStr, char *mac);


int SetLocalIp(char *ifName, const char *ipaddr);



#endif
