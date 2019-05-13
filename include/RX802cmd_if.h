#ifndef RX802_IF_H
#define RX802_IF_H

/* if want boardcast, set Ip 255.255.255.255 */

int RX802Cmd_Model_if(char *filepath);

int RX802Cmd_ReportState_if(char *ip);

int RX802Cmd_SetIp_if(const char *ip);

int RX802Cmd_Update_if(char *ip, char *filepath);


#endif