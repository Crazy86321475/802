#include <stdio.h>

int main()
{
    int i;
    int a = 1;
    int b;
    char mac[6] = {0x11, 0x1, 0x2, 0x3, 0x4, 0x5};
    for (i=0;i<6;++i){
        printf("%2hhx ", mac[i]);
    }
    return 0;
}
