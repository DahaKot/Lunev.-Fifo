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
const char PRODUCER[]   = "1";
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
    }                                                           \
    if (!have_pair) {                                           \
        /*if not successful: wait until croud_fifo*/            \
        /*is available or time_exceeded*/                       \
        close(croud_fifo_fd);                                   \
    }                                                           \
    else {                                                      \
        /*if successful: do not wait with sleep*/               \
        continue;                                               \
    }                                                           \
} while(0)

size_t buff_size = 1024;
int    read_n    = 2;

int main(int argc, char **argv) {
    if (argv == NULL || argc != 2) {
        printf("Ayayay!\n");
        return 0;
    }

    size_t PAGE_SIZE = sysconf(_SC_PAGE_SIZE);

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
    int data_fifo_fd     = open("data_fifo", O_WRONLY);
    int input_fd         = open(argv[1], O_RDONLY);
    lseek(input_fd, 0, SEEK_SET);

    char have_pair = 1;
    int read_s = 0;

    //open croud_fifo -> change its size -> if it's full: wait
    int croud_fifo_fd = open("croud_fifo2", O_RDWR | O_NONBLOCK);
    fcntl(croud_fifo_fd, F_SETPIPE_SZ, PAGE_SIZE);

    if (write(croud_fifo_fd, page_buff, PAGE_SIZE) != PAGE_SIZE) {
        have_pair = 0;
    }

    if (have_pair) {
        //initial signal for producer to start
        write(producer_fifo_fd, PRODUCER, read_n);
    }
    else {
        close(croud_fifo_fd);
    }

    while(1) {
        if (have_pair && read(producer_fifo_fd, fifo_buff, read_n) > 0) {
            read_s = read(input_fd, buff, buff_size);
            if (read_s < buff_size) {
                clear_rest(read_s, buff);
            }

            sleep_time = 1;

            write(data_fifo_fd, buff, buff_size);

            //allow consumer to work
            write(consumer_fifo_fd, CONSUMER, read_n);

            if (read_s < buff_size) {
                //EOF is reached
                break;
            }
        }
        else {
            if (!have_pair) {
                SET_HAVE_PAIR;
            }

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
