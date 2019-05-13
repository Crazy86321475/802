#include <stdio.h>
#include <string.h>

#include "RX802cmd_if.h"

#include "log_my.h"


#define TEST_HOST "222.24.86.181"


int main()
{
    int ret;
 #if 0
    ret = RX802Cmd_Update_if("192.168.1.20", "./test/DMX-803_1GRGB170.bin");
    if (ret == 0) {
        LOG("RX802Cmd_Update_if 192.168.1.20 ./test/DMX-803_1GRGB170.bin  ok");
    }
#endif
    //RX802Cmd_Update_if("255.255.255.255", "./test/ws-ws2811.bin");
    //RX802Cmd_ReportState_if(TEST_HOST);
#if 0
    ret = RX802Cmd_Model_if("./test/parax2.par");
    if (ret == 0) {
        LOG("RX802Cmd_Model_if ./test/para.par  ok");
    }
    RX802Cmd_SetIp_if("192.168.1.20");

    //RX802Cmd_Update_if("255.255.255.255", "./test/ws-ws2811.bin");
    #endif
    ret = RX802Cmd_Update_if("192.168.1.41", "./test/DMX-803_1GRGB170.bin");
    if (ret == 0) {
        LOG("RX802Cmd_Update_if 192.168.1.41 ./test/DMX-803_1GRGB170.bin  ok");
    }
    return 0;
}


