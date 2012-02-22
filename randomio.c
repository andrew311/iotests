/*
 * This code does random, direct to disk reads against a file.
 *
 * Compile
 *   gcc randomio.c -o randomio -O3
 * Usage
 *   Allocate a file with dd (this example allocates 1GB):
 *     dd if=/dev/zero of=myfile bs=1M count=1024
 *   Then run randomio against it:
 *      randomio myfile
 */
#define _GNU_SOURCE
#define _FILE_OFFSET_BITS 64
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int main(int argc, char *argv[])
{
    struct  stat sb;
    time_t  start_time = time(NULL), now, last_print_time = start_time;
    int     fd;
    size_t  buffer_size = 4096;
    char    *buffer;
    float   rate, elapsed;
    unsigned long count, max_ulong = -1;
    ssize_t num_read;
    off_t   offset, max_offset, offset_mask = ~(buffer_size - 1);

    // Allocate aligned memory
    if (0 != posix_memalign((void **)&buffer, buffer_size, buffer_size))
    {
        printf("failed posix_memalign\n");
        exit(1);
    }

    // Seed random number generator
    srand(start_time);

    // Open file and get stats
    fd = open(argv[1], O_DIRECT | O_SYNC | O_RDONLY);
    fstat(fd, &sb);
    max_offset = sb.st_size - buffer_size;

    // Print some information
    printf("file size: %llu bytes, %f MB\n", sb.st_size, (float)sb.st_size / 1024.0 / 1024.0);
    printf("sizeof(off_t):  %d\n", sizeof(off_t));
    printf("sb.st_blksize:  %lu\n", sb.st_blksize);
    printf("buffer_size:    %d\n", buffer_size);

    // Loop for a long time
    for(count = 1; count < max_ulong; count++)
    {
        // Calculate seek offset
        offset = ((float)rand() / (float)RAND_MAX) * sb.st_size;
        offset &= offset_mask;

        // Seek and read
        lseek(fd, offset, SEEK_SET);
        num_read = read(fd, buffer, buffer_size);

        // Periodically print status
        if (count % 1000 == 0)
        {
            now = time(NULL);
            elapsed = now - start_time;
            if ((now - last_print_time) >= 2)
            {
                rate = (float)count / elapsed;
                printf("now %lu, num_read %u, errno %d, offset %12llu, rate %.1f\n",
                    now, num_read, errno, offset, rate
                );
                last_print_time = now;
            }
        }
    }
}
