#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include <errno.h>
#define ERRNO strerror(errno)
#define LOG(fmt, ...) printf("[%s](%d):"fmt"\n", __func__, __LINE__, ##__VA_ARGS__)

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
	LOG("c:%hhx, invalid_char:%hhx", c, invalid_char);
    } while ((unsigned char)c == invalid_char);
    ftruncate(fd, off + 2);
    close(fd);
    return 0;
}

int main()
{
    deleteFileEndInvalidChar("./bin_file_test", 0);
    return 0;
}
