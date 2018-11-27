#define  _GNU_SOURCE

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

char  CONSUMER_TURN     = '0';
const char PRODUCER[]   = "1";
unsigned sleep_time     = 1;
unsigned MAX_SLEEP_TIME = 256;

void freee(void *ptr) {
    free(*(void **)ptr);
}

#define MKFIFO(name)                                            \
do {                                                            \
    if (mkfifo(name, 0644) != 0 && errno != EEXIST) {           \
        printf("Cant make fifo\n");                             \
        return 0;                                               \
    }}                                                          \
while(0)

#define SET_HAVE_PAIR                                           \
do {                                                            \
    croud_fifo_fd = open("croud_fifo3", O_RDWR | O_NONBLOCK);   \
    fcntl(croud_fifo_fd, F_SETPIPE_SZ, PAGE_SIZE);              \
    if (write(croud_fifo_fd, page_buff, PAGE_SIZE)              \
            == PAGE_SIZE) {                                     \
        have_pair = 1;                                          \
    }                                                           \
    if (!have_pair) {                                           \
        close(croud_fifo_fd);                                   \
    }                                                           \
    else {                                                      \
        continue;                                               \
    }}                                                          \
while(0)

#define D if(0)

size_t buff_size = 1024;

int main(int argc, char **argv) {
    if (argv == NULL || argc != 1) {
        printf("Ayayay!\n");
        return 0;
    }

    size_t PAGE_SIZE = sysconf(_SC_PAGE_SIZE);

    char *buff __attribute__((cleanup(freee)))
        = calloc(buff_size, sizeof(char));
    char *page_buff __attribute__((cleanup(freee)))
        = calloc(PAGE_SIZE, sizeof(char));

    errno = 0;
    MKFIFO("croud_fifo3");
    MKFIFO("produce_fifo");
    MKFIFO("consume_fifo");
    MKFIFO("data_fifo");

    int producer_fifo_fd = open("produce_fifo", O_RDWR | O_NONBLOCK);
    int consumer_fifo_fd = open("consume_fifo", O_RDWR | O_NONBLOCK);
    int data_fifo_fd  = open("data_fifo", O_RDONLY);

    char have_pair = 1;
    int read_s = 0;

    int croud_fifo_fd = open("croud_fifo3", O_RDWR | O_NONBLOCK);
    fcntl(croud_fifo_fd, F_SETPIPE_SZ, PAGE_SIZE);
    if (write(croud_fifo_fd, page_buff, PAGE_SIZE) != PAGE_SIZE) {
        have_pair = 0;
    }
    if (!have_pair) {
        close(croud_fifo_fd);
    }

    D printf("before while\n");

    while(1) {
        if (have_pair && read(consumer_fifo_fd, buff, 2) > 0) {
            D printf("inside critical section 1\n");
            sleep_time = 1;

            read_s = read(data_fifo_fd, buff, buff_size);
            D printf("read from data fifo:\n %s\n", buff);

            write(producer_fifo_fd, PRODUCER, 2);
            D printf("written to producer fifo\n");

            write(STDOUT_FILENO, buff, read_s);
            if (buff[buff_size - 1] == EOF) {
                D printf("ended\n");
                break;
            }
        }
        else {
            if (!have_pair) {
                D printf("do not have pair\n");
                SET_HAVE_PAIR;
            }
            D printf("sleep\n");
            sleep(sleep_time);
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
    D printf("closed all fifos\n");

    return 0;
}
