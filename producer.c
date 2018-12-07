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

const size_t buff_size = 1024;

int main(int argc, char **argv) {
    if (argv == NULL || argc != 2) {
        printf("Ayayay!\n");
        return 0;
    }

    size_t PAGE_SIZE = sysconf(_SC_PAGE_SIZE);

    char *buff[buff_size];
    char *page_buff[PAGE_SIZE];

    errno = 0;
    MKFIFO("croud_fifo2");
    MKFIFO("data_fifo");

    int input_fd = open(argv[1], O_RDONLY);
    if (input_fd == -1) {
        printf("Cannot open the file\n");
        return 0;
    }
    lseek(input_fd, 0, SEEK_SET);

    int read_s = 0;
    int data_fifo_fd = 0;

    //open croud_fifo -> change its size -> if it's full: wait
    int croud_fifo_fd = open("croud_fifo2", O_RDWR | O_NONBLOCK);
    fcntl(croud_fifo_fd, F_SETPIPE_SZ, PAGE_SIZE);

    if (write(croud_fifo_fd, page_buff, PAGE_SIZE) != PAGE_SIZE) {
        return 0;
    }

    data_fifo_fd = open("data_fifo", O_WRONLY);

    while(1) {
        read_s = read(input_fd, buff, buff_size);

        write(data_fifo_fd, buff, read_s);

        if (read_s < buff_size) {
            //EOF is reached
            break;
        }
    }

    return 0;
}
