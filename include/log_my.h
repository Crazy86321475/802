#ifndef LOG_MY_H
#define LOG_MY_H


#include <errno.h>

#define ERRNO strerror(errno)
#define LOG(fmt, ...) printf("[%s](%d):"fmt"\n", __func__, __LINE__, ##__VA_ARGS__)

#endif
