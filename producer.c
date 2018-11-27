#define  _GNU_SOURCE

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

char  PRODUCER_TURN     = '1';
const char CONSUMER[]   = "0";
unsigned sleep_time     = 1;
unsigned MAX_SLEEP_TIME = 256;

void clear_rest (int read_s, char *mem_p);
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
    croud_fifo_fd = open("croud_fifo2", O_RDWR | O_NONBLOCK);   \
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
    if (argv == NULL || argc != 2) {
        printf("Ayayay!\n");
        return 0;
    }

    size_t PAGE_SIZE = sysconf(_SC_PAGE_SIZE);

    int input_fd = open(argv[1], O_RDONLY);

    char *buff __attribute__((cleanup(freee)))
        = calloc(buff_size, sizeof(char));
    char *fifo_buff __attribute__((cleanup(freee)))
        = calloc(buff_size, sizeof(char));
    char *page_buff __attribute__((cleanup(freee)))
        = calloc(PAGE_SIZE, sizeof(char));

    errno = 0;
    MKFIFO("croud_fifo2");
    MKFIFO("produce_fifo");
    MKFIFO("consume_fifo");
    MKFIFO("data_fifo");

    int producer_fifo_fd = open("produce_fifo", O_RDWR | O_NONBLOCK);
    int consumer_fifo_fd = open("consume_fifo", O_RDWR | O_NONBLOCK);
    int data_fifo_fd  = open("data_fifo", O_WRONLY);

    char have_pair = 1;
    int read_s = 0;

    int croud_fifo_fd = open("croud_fifo2", O_RDWR | O_NONBLOCK);
    fcntl(croud_fifo_fd, F_SETPIPE_SZ, PAGE_SIZE);
    if (write(croud_fifo_fd, page_buff, PAGE_SIZE) != PAGE_SIZE) {
        have_pair = 0;
    }

    if (have_pair) {
        write(producer_fifo_fd, "1", 2);
    }
    else {
        close(croud_fifo_fd);
    }
    lseek(input_fd, 0, SEEK_SET);

    D printf("before while\n");

    while(1) {
        if (have_pair && read(producer_fifo_fd, fifo_buff, 2) > 0) {
            read_s = read(input_fd, buff, buff_size);
            if (read_s < buff_size) {
                clear_rest(read_s, buff);
            }

            D printf("read-ed from file %d\n", read_s);

            D printf("inside critical section 1\n");
            sleep_time = 1;
            write(data_fifo_fd, buff, buff_size);
            D printf("written to data fifo:\n %s\n", buff);

            write(consumer_fifo_fd, CONSUMER, 2);
            D printf("written to consumer fifo\n");

            if (read_s < buff_size) {
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

    return 0;
}

void clear_rest (int read_s, char *mem_p) {
    for ( ; read_s < buff_size; read_s++ ) {
        mem_p[read_s] = '\0';
    }
    mem_p[buff_size - 1] = EOF;
}
