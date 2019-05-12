#include <stdio.h>
#include <string.h>

#include "RX802cmd_if.h"

#include "log_my.h"


#define TEST_HOST "222.24.86.181"


int main()
{

    //RX802Cmd_Update_if("255.255.255.255", "./test/DMX-803_1GRGB170.bin");
    //RX802Cmd_ReportState_if(TEST_HOST);
    //RX802Cmd_Model_if("./test/parax2.par");
    RX802Cmd_SetIp_if("192.168.1.20");

    return 0;
}


