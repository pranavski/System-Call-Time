#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {

    void *shm_ptr;
    int size;
    int shm_fd;
    char *shm_name;
    struct timeval *starting_time;

    if(argc < 2){
        fprintf(stderr, "Usage: %s <command>\n", argv[0]);
        return -1;
    }

    shm_name = "shared_memory";
    size = sizeof(struct timeval);

    /* create the shared memory segment */
    /* from the example given */
    shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, 0644);
    if (shm_fd == -1) {
        fprintf(stderr, "Error unable to create shared memory, '%s', errno = %d (%s)\n", shm_name,
                errno, strerror(errno));
        return -1;
    }

    /* configure the size of the shared memory segment */
    /* from the example given */
    if (ftruncate(shm_fd, size) == -1) {
         fprintf(stderr, "Error configure create shared memory, '%s', errno = %d (%s)\n", shm_name,
                errno, strerror(errno));
         shm_unlink(shm_name);
         return -1;
    }

    /* now map the shared memory segment in the address space of the process */
    /* from the example given */
    shm_ptr = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        fprintf(stderr, "Error: unable to map shared memory segment, errno = %d (%s) \n",
                errno, strerror(errno));
        return -1;
    }

    starting_time = (struct timeval *)shm_ptr;

    //forking a child process
    pid_t pid = fork();
    if (pid < 0) {
        perror ("fork");
        return -1;
    }
    else if (pid == 0) {
        if (execvp(argv[1], &argv[1]) == -1) {
            perror("execvp");
            exit(1);
        }
    }
    else {
        struct timeval end_time;
        int status;
        gettimeofday(starting_time, NULL);

        if (waitpid(pid, &status, 0) != -1 && WIFEXITED(status)) {
            gettimeofday(&end_time, NULL);
            double time_elapsed = (end_time.tv_sec - starting_time->tv_sec) + (end_time.tv_usec - starting_time->tv_usec) / 1000000.0;
            printf("Elapsed Time: %.5f seconds\n", time_elapsed);
        }
    }

    if (munmap(shm_ptr, size) == -1) {
        fprintf(stderr, "Error unmapping shared memory segment '%s', errno = %d (%s) \n", shm_name,
                errno, strerror(errno));
        return -1;
    }

    if(shm_unlink(shm_name) == -1) {
        fprintf(stderr, "Error unable to remove shared memory segment '%s', errno = %d (%s) \n", shm_name,
                errno, strerror(errno));
        return -1;
    }
    
    return 0;
}
