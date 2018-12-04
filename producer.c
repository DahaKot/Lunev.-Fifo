#define  _GNU_SOURCE

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

unsigned sleep_time     = 512;
unsigned MAX_SLEEP_TIME = 1024 * 1024 * 2;

void freee(void *ptr) {
    free(*(void **)ptr);
}

#define MKFIFO(name)                                            \
do {                                                            \
    if (mkfifo(name, 0644) != 0 && errno != EEXIST) {           \
        printf("Cant make fifo\n");                             \
        return 0;                                               \
    }                                                           \
} while(0)

//try to capture croud_fifo
#define SET_HAVE_PAIR                                           \
do {                                                            \
    croud_fifo_fd = open("croud_fifo2", O_RDWR | O_NONBLOCK);   \
    fcntl(croud_fifo_fd, F_SETPIPE_SZ, PAGE_SIZE);              \
    if (write(croud_fifo_fd, page_buff, PAGE_SIZE)              \
            == PAGE_SIZE) {                                     \
        have_pair = 1;                                          \
        /*if successful: open data_fifo*/                       \
        data_fifo_fd = open("data_fifo", O_WRONLY);             \
    }                                                           \
    if (!have_pair) {                                           \
        /*if not successful: wait until croud_fifo*/            \
        /*is available or time_exceeded*/                       \
        close(croud_fifo_fd);                                   \
    }                                                           \
} while(0)

const size_t buff_size = 1024;
const int    read_n    = 2;

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
    MKFIFO("produce_fifo");
    MKFIFO("consume_fifo");
    MKFIFO("data_fifo");

    int input_fd = open(argv[1], O_RDONLY);
    if (input_fd == -1) {
        printf("Cannot open the file\n");
        return 0;
    }
    lseek(input_fd, 0, SEEK_SET);

    char have_pair = 1;
    int read_s = 0;
    int data_fifo_fd = 0;

    //open croud_fifo -> change its size -> if it's full: wait
    int croud_fifo_fd = open("croud_fifo2", O_RDWR | O_NONBLOCK);
    fcntl(croud_fifo_fd, F_SETPIPE_SZ, PAGE_SIZE);

    if (write(croud_fifo_fd, page_buff, PAGE_SIZE) != PAGE_SIZE) {
        have_pair = 0;
        close(croud_fifo_fd);
        //return 0;
    }

    int consumer_fifo_fd = open("consume_fifo", O_RDWR);
    fcntl(consumer_fifo_fd, F_SETPIPE_SZ, PAGE_SIZE);
    write(consumer_fifo_fd, page_buff, PAGE_SIZE);

    int producer_fifo_fd = open("produce_fifo", O_RDONLY);
    fcntl(producer_fifo_fd, F_SETPIPE_SZ, PAGE_SIZE);
    read(producer_fifo_fd, page_buff, PAGE_SIZE);

    if (have_pair) {
        data_fifo_fd = open("data_fifo", O_WRONLY);
    }

    while(1) {
        if (have_pair) {
            read_s = read(input_fd, buff, buff_size);

            write(data_fifo_fd, buff, read_s);

            if (read_s < buff_size) {
                //EOF is reached
                break;
            }
        }
        else {
            SET_HAVE_PAIR;

            usleep(sleep_time);
            sleep_time <<= 1;
            if (sleep_time > MAX_SLEEP_TIME) {
                printf("I was waiting too long\n");
                break;
            }
        }
    }

    close(croud_fifo_fd);
    close(producer_fifo_fd);
    close(consumer_fifo_fd);
    close(data_fifo_fd);

    return 0;
}
