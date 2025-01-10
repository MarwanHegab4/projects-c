#include "signals.h"
#include "core.h"
#include <errno.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

void no_interrupt_sleep(int sec) {
    struct timespec req, rem;
    req.tv_sec = sec;
    req.tv_nsec = 0;
    while (nanosleep(&req, &rem) == -1) {
        if (errno == EINTR) {
            req = rem;
        } else {
            perror("nanosleep failed");
            exit(EXIT_FAILURE);
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <core_id> <read_fd> <write_fd> <max_sleep_time>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int core_id = atoi(argv[1]);

    if (core_id < 1 || core_id > 3) {
        fprintf(stderr, "Error: Core ID must be between 1 and 3.\n");
        exit(EXIT_FAILURE);
    }

    int read_fd = atoi(argv[2]);
    int write_fd = atoi(argv[3]);
    int max_sleep_time = atoi(argv[4]);

    // Determine signal based on core_id
    int signal_send;
    switch(core_id) {
        case 1: signal_send = SIGNAL_CORE_1; break;
        case 2: signal_send = SIGNAL_CORE_2; break;
        case 3: signal_send = SIGNAL_CORE_3; break;
        default:
            fprintf(stderr, "Invalid core ID.\n");
            exit(EXIT_FAILURE);
    }

    printf("Core %d started and waiting for tasks.\n", core_id);
    fflush(stdout);
    srand(time(NULL) + core_id);
    int task_id;
    int task_count = 0;
    while (1) {
        ssize_t bytes_read = read(read_fd, &task_id, sizeof(task_id));
        if (bytes_read > 0) {
            printf("Core %d received task ID: %d\n", core_id, task_id);
            fflush(stdout);

            int sleep_time = rand() % max_sleep_time + 1; 
            no_interrupt_sleep(sleep_time);

            // send result to process
            if (write(write_fd, &task_id, sizeof(task_id)) == -1) {
                perror("write to core_to_main failed");
                exit(EXIT_FAILURE);
            }

            // send signal to process
            if (kill(getppid(), signal_send) == -1) {
                perror("kill failed");
                exit(EXIT_FAILURE);
            }

            task_count++;
            printf("Core %d completed task ID: %d, task_count=%d\n", core_id, task_id, task_count);
            fflush(stdout);
        } else if (bytes_read == 0) { // EOF, no more tasks
            break;

        } else {
            if (errno == EINTR) { // interrupted by signal
                continue; 
            }
            perror("read from main_to_core failed");
            exit(EXIT_FAILURE);
        }
    }
    printf("Core %d processed %d tasks.\n", core_id, task_count);
    fflush(stdout);
    close(read_fd);
    close(write_fd);
    exit(0);
}

