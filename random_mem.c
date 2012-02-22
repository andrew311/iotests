/*
 * This code does random, direct to disk reads against a file.
 *
 * Compile
 *   gcc random_mem.c -o random_mem -O3
 * Usage
 *   random_mem {number of MB to allocate}
 */
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
    time_t  start_time = time(NULL), now, last_print_time = start_time;
    size_t  offset, buffer_size = atoi(argv[1]) * 1024 * 1024;
    char    *buffer, value;
    float   rate, elapsed;
    unsigned long count, max_ulong = -1;

    // Allocate aligned memory
    if (0 != posix_memalign((void **)&buffer, 4096, buffer_size))
    {
        printf("failed posix_memalign\n");
        exit(1);
    }

    // Seed random number generator
    srand(start_time);

    // Print some information
    printf("buffer_size:    %d\n", buffer_size);
    printf("sizeof(size_t): %d\n", sizeof(size_t));

    // Loop for a long time
    for(count = 1; count < max_ulong; count++)
    {
        // Calculate seek offset
        offset = ((float)rand() / (float)RAND_MAX) * buffer_size;

        // Read value from memory, add to it just so work is done
        buffer[offset] = buffer[offset] + (char)1;

        // Periodically print status
        if (count % 1000000 == 0)
        {
            now = time(NULL);
            elapsed = now - start_time;
            if ((now - last_print_time) >= 2)
            {
                rate = (float)count / elapsed;
                printf("now %lu, value %u, offset %12u, rate %.1f\n",
                    now, value, offset, rate
                );
                last_print_time = now;
            }
        }
    }
}
