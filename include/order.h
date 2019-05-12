

#ifndef ORDER_H
#define ORDER_H




/* check order*/
UINT16 order_buf = 1;
#define IS_LE (UINT8)order_buf //is little order?  
#define IS_BE !((UINT8)order_buf)

char IsLittleEndian()
{
    short buf = 1; //0x0001
    return (char)buf;
} 

char IsBigEndian()
{
    return !IsLittleEndian();
}



#endif
