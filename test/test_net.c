#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <fcntl.h>



#include "../include/pc2rx802_ptcl.h"

#include <errno.h>
#define ERRNO strerror(errno)
#define LOG(fmt, ...) printf("[%s](%d):"fmt"\n", __func__, __LINE__, ##__VA_ARGS__)


//#define PORT_TYPE_SEND 0xeeee 
#define MAXDATASIZE 1024

#define LOCAL_IP "222.24.86.181"

#define GET_ADDR(x) ((x[0]<<16) | (x[1]<<8) |(x[2]))

#define FILE_NAME_MAXLEN 64
//static int update_reset_flag = 1;
static char update_file[FILE_NAME_MAXLEN] = {0};
static char par_file[FILE_NAME_MAXLEN] = {0};

static unsigned char par_file_ip[4] = {0x0a, 0x01, 0xa8, 0xc0};

typedef enum {
    UPDATE_FILE = 0,
    PAR_FILE,

    RESERVE_FILE
}FILE_TYPE;

int getFileNameOfTime(char *update_file, FILE_TYPE type)
{
    time_t tNow =time(NULL); ;
    struct tm* ptm = localtime(&tNow);
    strftime(update_file, 50, "%H-%M-%S", ptm);
    switch (type)
    {
    case UPDATE_FILE:
        strcat(update_file, "_update.bin");
        break;
    case PAR_FILE:
        strcat(update_file, ".par");
        break;
    default:
        ;
    }
    return 0;
}

int buf2file(char *buf, UINT32 len, char *update_file)
{

    FILE *fd = fopen(update_file, "ab");  //binary file is "ab"
    if (!fd) {
        LOG("fopen file err!");
        return -1;
    }
    fwrite(buf, 1, len, fd);
    fclose(fd);
    return 0;
}

int deleteFileEndInvalidChar(char *filepath, unsigned char invalid_char)
{
    int fd = open(filepath, O_RDWR);
    if (fd == -1) {
        LOG("open err!");
        return -1; 
    }   
    off_t off = lseek(fd, 0, SEEK_END);
    printf("end off:%ld\n", off);
    int ret = -1; 
    char c = 0;
    ret = read(fd, &c, 1); 
    off = lseek(fd, -1, SEEK_CUR);
    do {
        ret = read(fd, &c, 1); 
        off = lseek(fd, -2, SEEK_CUR);
    } while ((unsigned char)c == invalid_char);
    ftruncate(fd, off + 2);                                                                                                                                                                
    close(fd);
    return 0;
}


int RX802_RCVE(char *buf, UINT32 len)
{
    LOG("get cmd %hhx", buf[0]);
    UINT8 *p_mac;
    UINT32 addr;
    in_addr_t local_ip = inet_addr(LOCAL_IP);
    //void *p = (void *)buf;
    if (len == RX802_UDP_MAX_PKT_LEN) {
        LOG("len ok!");
    } else {LOG("len err!(%u)", len);}
    switch (buf[0])
    {
    case PKT_SET_IP:
        p_mac = ((PKT_SET_IP_DATA *)buf)->mac;
        LOG("set mac %hhx %hhx %hhx %hhx %hhx %hhx , ip %s",
            p_mac[0],p_mac[1],p_mac[2],p_mac[3],p_mac[4],p_mac[5],
            inet_ntoa(*(struct in_addr*)((PKT_SET_IP_DATA *)buf)->ip));
        break;
    case PKT_UPDATE_ONLINE:
        addr = GET_ADDR(((PKT_UPDATE_ONLINE_DATA *)buf)->addr);
        if (!strlen(update_file)) {
            getFileNameOfTime(update_file, UPDATE_FILE);
        }
        LOG("file offset :%u", addr);
        if (addr == 0xffffff) {
            LOG("get update bin over!");
            deleteFileEndInvalidChar(update_file, 0xff);
            memset(update_file, 0, sizeof(update_file));
        } else {
            buf2file(((PKT_UPDATE_ONLINE_DATA *)buf)->bin_data,
                     sizeof(((PKT_UPDATE_ONLINE_DATA *)buf)->bin_data),
                     update_file);
        }
        break;
//    case PKT_DISPLAY:
//        LOG("TODO PKT_DISPLAY: please!");
        break;
    case PKT_CONTROL:
        LOG("TODO PKT_CONTROL: please!");
        break;
    case PKT_MODEL:
        if (!strlen(par_file)) {
            getFileNameOfTime(par_file, PAR_FILE);
        }

        LOG("file offset :%u", addr);
        if ((unsigned char)buf[2] == 0xff) {
            LOG("get par file over!");
            //deleteFileEndInvalidChar(par_file, 0xff);
            memset(par_file, 0, sizeof(par_file));
        } else {
            buf2file((char*)par_file_ip, sizeof(local_ip), par_file);
            buf2file(buf, sizeof(*((PKT_MODEL_DATA *)buf)), par_file);
        }
        break;
//    case PKT_FRAME_SYN:
//        LOG("TODO: please!");
        break;
//    case PKT_STOP_PLAY:
//        LOG("TODO: please!");
//        break;
    case PKT_REPORT_STATE:
        LOG("TODO: please!");
        break;
    default:
    ;
    }
    return 0;
}



int main(void)
{
    int sockfd;
    struct sockaddr_in server;
    struct sockaddr_in client;
    socklen_t len;
    int num;
    memset(update_file, 0, sizeof(update_file));
    char buf[MAXDATASIZE];
    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        perror("Creating socket failed.\n");
        exit(1);
    }
    bzero(&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT_TYPE_SEND);
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    if(bind(sockfd, (struct sockaddr *)&server, sizeof(server)) == -1)
    {
        perror("Bind() error.\n");
        exit(1);
    }

    len = sizeof(client);
    while(1)
    {
        LOG("recving...");
        num = recvfrom(sockfd, buf, MAXDATASIZE, 0, (struct sockaddr *)&client, &len);
        if(num < 0) {
            perror("recvfrom() error.\n");
            exit(1);
        }
        buf[num] = '\0';
        printf("You got a message from client. \nIt's ip is %s, port is %d. \n",
                inet_ntoa(client.sin_addr),(int)htons(client.sin_port));
        RX802_RCVE(buf, num);
        if (buf[0] == PKT_REPORT_STATE) {
            msleep(5000);
            LOG("Get PKT_REPORT_STATE, sending responed!");
            sendto(sockfd, "Get PKT_REPORT_STATE", 20, 0, (struct sockaddr *)&client, sizeof(client));
        }
    }
    close(sockfd);
}
