
#ifndef PC2RX802_PTCL
#define PC2RX802_PTCL

#include "UDP_demo.h"


typedef unsigned char UINT8;
typedef unsigned short UINT16;
typedef unsigned int UINT32;

#define RX802_UDP_MAX_PKT_LEN   773
/* device rx802 udp port define */
#define PORT_TYPE_SEND 0xeeee  //61166
#define PORT_TYPE_RECV 0xea60 //60000

/* delay time (ms)*/
#define RX802_UDP_TRSF_STATE_FAST_PKT    0xc7  //0xc7
#define RX802_UDP_TRSF_STATE_ALL_PKT     0x1ed //0xff
#define RX802_UDP_SHORT_SLEEP   50
#define RX802_UDP_LONG_SLEEP    3000

/* cmd reference */



typedef enum UDP_PACKET_TYPE_RX802
{
    PKT_INVALID = 0,
    PKT_SET_IP = 0x10,
    PKT_UPDATE_ONLINE = 0x20,
    PKT_DISPLAY = 0x30,
    PKT_CONTROL = 0x31,
    PKT_MODEL = 0x32,
    PKT_FRAME_SYN = 0x33,
    PKT_STOP_PLAY = 0x34,
    PKT_REPORT_STATE = 0x35,
    PKT_RESERVE
}UDP_PACKET_TYPE_RX802;

typedef struct RX802_UDP_PROTOCOL_PARAM
{
    UPUSP upusp;
    // may be hava some pravite?
    UDP_PACKET_TYPE_RX802 type;
    UINT8 have_respond;
}UPP;

/* Data in packet all use Big endian.
*/

//    PKT_INVALID = 0,
//----------------------------------------------
//    PKT_SET_IP = 0x10,
#define RX802_PKT_SET_IP_DATA_LEN 12
typedef struct PKT_SET_IP_DATA
{
    UINT8 cmd;
    UINT8 mac[6];
    UINT8 ip[4];
    //char end;    //use 'E'
}PKT_SET_IP_DATA;

//----------------------------------------------
//    PKT_UPDATE_ONLINE = 0x20,
#define UPDATE_PKT_SIZE         256
typedef struct PKT_UPDATE_ONLINE_DATA
{
    UINT8 cmd;
    UINT8 addr[3];
    UINT8 bin_data[UPDATE_PKT_SIZE];
}PKT_UPDATE_ONLINE_DATA;

//----------------------------------------------
//    PKT_DISPLAY = 0x30,
#define RX802_PKT_DISPLAY_DATA_LEN  771
#define RX802_PKT_PIXEL_RGB_DATA_LEN  768

typedef struct PKT_DISPLAY_DATA
{
    UINT8 cmd;
    UINT8 sequence[2];
    UINT8 pix_rgb_data[RX802_PKT_PIXEL_RGB_DATA_LEN];
}PKT_DISPLAY_DATA;

//----------------------------------------------
//    PKT_CONTROL = 0x31,
#define RX802_PKT_CONTROL_DATA_LEN  269
typedef struct PKT_CONTROL_DATA
{
    UINT8 cmd;
    UINT8 save;
    UINT8 reserve_0[2];
    UINT8 ctrl_type;
    UINT8 clk_rate;

    UINT8 t0h;
    UINT8 t0l;
    UINT8 t1h;
    UINT8 t1l;

    UINT8 gray;

    UINT8 bri_r;
    UINT8 bri_g;
    UINT8 bri_b;

    UINT8 rgb_pin_valid;
    UINT8 rgb_pin[256];
}PKT_CONTROL_DATA;

//----------------------------------------------
//    PKT_MODEL = 0x32,

#define PAR_FILE_MODEL_BLOCK        512 //508 to send;  512-4, 4:ip
#define PAR_PKT_MODEL_DATA_LEN      508 //508 to send;  512-4, 4:ip
//508{cmd[1], save[1], reserve_0[1], port_addr[1], model_data[504]}
#define PKT_MODEL_SIZE              504 

typedef struct PKT_MODEL_DATA
{
    UINT8 cmd;
    UINT8 save;
    UINT8 reserve_0; //except last pkt ,  [0x32, 0xff, 0xff ... 0xff]
    UINT8 port_addr;
    UINT8 model_data[PKT_MODEL_SIZE];
}PKT_MODEL_DATA;

//----------------------------------------------
//    PKT_FRAME_SYN = 0x33,
#define RX802_PKT_FRAME_SYN_DATA_LEN 771
#define RX802_PKT_GAMMAM_TAB_LEN 768

typedef struct PKT_FRAME_SYN_DATA
{
    UINT8 cmd;
    UINT8 brightness;
    UINT8 rgb_gamma;
    UINT8 gamma_tab[RX802_PKT_GAMMAM_TAB_LEN];
}PKT_FRAME_SYN_DATA;

//----------------------------------------------
//    PKT_STOP_PLAY = 0x34,
#define RX802_PKT_STOP_PLAY_DATA_LEN 1 
typedef struct PKT_STOP_PLAY_DATA
{
    UINT8 cmd;
}PKT_STOP_PLAY_DATA;

//----------------------------------------------
//    PKT_REPORT_STATE = 0x35,
#define RX802_PKT_REPORT_STATE_DATA_LEN 1
typedef struct PKT_REPORT_STATE_DATA
{
    UINT8 cmd;
}PKT_REPORT_STATE_DATA;
//respond
#define RX802_PKT_REPORT_STATE_RESPOND_DATA_LEN 6
typedef struct PKT_REPORT_STATE_DATA_RESPOND
{
    UINT8 cmd;
    UINT8 ctrl_type;
    UINT8 ctrl_clk;
    UINT8 brightness;
    UINT8 temperature[2];
}PKT_REPORT_STATE_DATA_RESPOND;


void pktAddchecksum(char *pkt);

int RX802Cmd2PktBuff(void *cmd_buf, UPUSP *upusp, UPP *upp, char *pkt_buf);


#endif
