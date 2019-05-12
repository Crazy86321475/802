
#include <stdio.h>
#include <string.h>

#include "pc2rx802_ptcl.h"

#include "order.h"

#include "log_my.h"

void pktAddchecksum(char *pkt)
{
    pkt[261] = 0xfd;
    pkt[262] = 0x54;
    pkt[263] = 0x88;
    pkt[264] = 0x91;
}



static int ParseBufAndFillPkt(void *struct_buf, char *pkt_buf, UPUSP *upusp, UPP *upp)
{
    UPUSP *p_upp = &upp->upusp;
    p_upp->port =  PORT_TYPE_SEND; // set defult
    strcpy(p_upp->ip, upusp->ip);
    upp->type = ((char*)(struct_buf))[0];
    LOG("recv cmd buff :%hhx", upp->type);
    switch (upp->type)
    {
    case PKT_SET_IP:
        if (strcmp(BOARDCAST_STR_IP, p_upp->ip)) {
            LOG("PKT_SET_IP p_upp->ip:%s should be %s.", p_upp->ip, BOARDCAST_STR_IP);
            return -1;
        }
        memcpy(pkt_buf, struct_buf, sizeof(PKT_SET_IP_DATA));
        pktAddchecksum(pkt_buf);
        break;
    case PKT_UPDATE_ONLINE:
        memcpy(pkt_buf, struct_buf, sizeof(PKT_UPDATE_ONLINE_DATA));
        pktAddchecksum(pkt_buf);
        break;
    case PKT_DISPLAY:
        memcpy(pkt_buf, struct_buf, sizeof(PKT_DISPLAY_DATA));
        break;
    case PKT_CONTROL:
        memcpy(pkt_buf, struct_buf, sizeof(PKT_CONTROL_DATA));
        pktAddchecksum(pkt_buf);
        break;
    case PKT_MODEL:
        memcpy(pkt_buf, struct_buf, sizeof(PKT_MODEL_DATA));
        pktAddchecksum(pkt_buf);
        break;
    case PKT_FRAME_SYN:
        memcpy(pkt_buf, struct_buf, sizeof(PKT_FRAME_SYN_DATA));
        break;
    case PKT_STOP_PLAY:
        memcpy(pkt_buf, struct_buf, sizeof(PKT_STOP_PLAY_DATA));
        break;
    case PKT_REPORT_STATE:
        memcpy(pkt_buf, struct_buf, sizeof(PKT_REPORT_STATE_DATA));
        break;
        //case PKT_RESERVE:
    default:
        LOG("error: get invalid cmd");
        return -1;
    }
    return 0;
}

int RX802Cmd2PktBuff(void *cmd_buf, UPUSP *upusp, UPP *upp, char *pkt_buf)
{
    /* firstly get udp options and rx802 prtcl with cmd of this pkt */
    /* secondly, check with upusp */
    if (ParseBufAndFillPkt(cmd_buf, pkt_buf, upusp, upp)) { 
        memset(pkt_buf, 0, RX802_UDP_MAX_PKT_LEN);
        return -1;
    }
    return 0;
}



