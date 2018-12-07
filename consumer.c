#define  _GNU_SOURCE

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#define MKFIFO(name)                                            \
do {                                                            \
    if (mkfifo(name, 0644) != 0 && errno != EEXIST) {           \
        printf("Cant make fifo\n");                             \
        return 0;                                               \
    }                                                           \
} while(0)

const size_t buff_size  = 1024;

int main(int argc, char **argv) {
    if (argv == NULL || argc != 1) {
        printf("Ayayay!\n");
        return 0;
    }

    size_t PAGE_SIZE = sysconf(_SC_PAGE_SIZE);

    char *buff[buff_size];
    char *page_buff[PAGE_SIZE];

    errno = 0;
    MKFIFO("croud_fifo3");
    MKFIFO("data_fifo");

    int read_s = 0;
    int data_fifo_fd = 0;

    //open croud_fifo -> change its size -> if it's full: wait
    int croud_fifo_fd = open("croud_fifo3", O_RDWR | O_NONBLOCK);
    fcntl(croud_fifo_fd, F_SETPIPE_SZ, PAGE_SIZE);

    if (write(croud_fifo_fd, page_buff, PAGE_SIZE) != PAGE_SIZE) {
        return 0;
    }

    data_fifo_fd = open("data_fifo", O_RDONLY);

    while(1) {
        read_s = read(data_fifo_fd, buff, buff_size);

        write(STDOUT_FILENO, buff, read_s);

        if (read_s < buff_size) {
            break;
        }
    }

    return 0;
}
