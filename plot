//does not work :(
fcntl (fd, F_GETPIPE_SZ)
2 proccess:
1st receives filename and send file to the 2nd one
2nd receives file from 1st and print it to the STDOUT

fifos:
0 sending data
1 can a proccess access to the fifo0 or not(between cons. and prod.)
2 can a proccess execute or not (between consumers/producers)

//consumer:
open fifo3 for write
write to full fifo3 (non_block)
set_have_pair()
if !have_pair:
    close fifo3
while(1):
    if have_pair && read(fifo1) == 0:
        get data from fifo0 (non_block)
        write to fifo1 1
        if end:
            break
    else:
        if not_have_pair:
            open fifo3 for write
            write to full fifo3 (non_block)
            set_have_pair()
            if !have_pair:
                close fifo3
        smart_sleep
set_have_pair() :
    if write successful:
        have_pair = 1
    else:
        have_pair = 0

//producer:
open fifo2 for write
write to full fifo3 (non_block)
set_have_pair()
if !have_pair:
    close fifo2
while(1):
    if have_pair && read(fifo1) == 1:
        put data to fifo0 (non_block)
        write to fifo1 0
        if end:
            break
    else:
        if not_have_pair:
            open fifo2 for write
            write to full fifo2 (non_block)
            set_have_pair()
            if !have_pair:
                close fifo2
        smart_sleep
