#include "signals.h"
#include "core.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <errno.h>
#include <signal.h>
#include <string.h>

#define NUM_CORES 3

typedef struct {
    int main_to_core[2];  
    int core_to_main[2];  
    pid_t pid;            
    int status;           
    int signal_num;       
} Core;

// variables used to update within signal handlers
volatile sig_atomic_t msg_num_core[NUM_CORES] = {0, 0, 0};


// signal handler for new message from core
void handle_new_msg_from_core(int sig) {
    for (int i = 0; i < NUM_CORES; i++) {
        if (sig == (SIGNAL_CORE_1 + i)) {
            msg_num_core[i]++;
            printf("Received SIGNAL_CORE_%d from Core %d\n", i + 1, i + 1);
            fflush(stdout);
            break;
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <total_tasks> <max_sleep_time>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *endptr;
    long total_tasks_long = strtol(argv[1], &endptr, 10);
    if (*endptr != '\0' || total_tasks_long <= 0) {
        fprintf(stderr, "Error: <total_tasks> must be a positive integer.\n");
        exit(EXIT_FAILURE);
    }
    int total_tasks = (int)total_tasks_long;

    long max_sleep_time_long = strtol(argv[2], &endptr, 10);
    if (*endptr != '\0' || max_sleep_time_long <= 0) {
        fprintf(stderr, "Error: <max_sleep_time> must be a positive integer.\n");
        exit(EXIT_FAILURE);
    }
    int max_sleep_time = (int)max_sleep_time_long;

    srand(time(NULL));

    // initialize core structures and create pipes
    Core cores[NUM_CORES];
    for (int i = 0; i < NUM_CORES; i++) {
        cores[i].signal_num = SIGNAL_CORE_1 + i;
        if (pipe(cores[i].main_to_core) == -1 || pipe(cores[i].core_to_main) == -1) {
            perror("Pipe creation failed");
            exit(EXIT_FAILURE);
        }
    }

    // signal handlers for each core
    struct sigaction sa;
    sa.sa_handler = handle_new_msg_from_core;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    for (int i = 0; i < NUM_CORES; i++) {
        if (sigaction(cores[i].signal_num, &sa, NULL) == -1) {
            perror("Error registering signal handler");
            exit(EXIT_FAILURE);
        }
    }

    // fork and exec cores
    for (int i = 0; i < NUM_CORES; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("Fork failed");
            exit(EXIT_FAILURE);

        }
        if (pid == 0) {
            
             // close unused pipe ends
            for (int j = 0; j < NUM_CORES; j++) {
                if (j != i) {
                    close(cores[j].main_to_core[0]);
                    close(cores[j].main_to_core[1]);
                    close(cores[j].core_to_main[0]);
                    close(cores[j].core_to_main[1]);
                }
            }

            close(cores[i].main_to_core[1]);
            close(cores[i].core_to_main[0]);

            char read_fd_str[10];
            char write_fd_str[10];
            snprintf(read_fd_str, sizeof(read_fd_str), "%d", cores[i].main_to_core[0]);
            snprintf(write_fd_str, sizeof(write_fd_str), "%d", cores[i].core_to_main[1]);

            char core_id_str[2];
            snprintf(core_id_str, sizeof(core_id_str), "%d", i + 1);

            char max_sleep_time_str[10];
            snprintf(max_sleep_time_str, sizeof(max_sleep_time_str), "%d", max_sleep_time);
    
            execl("./core", "core", core_id_str, read_fd_str, write_fd_str, max_sleep_time_str, (char *)NULL);
            perror("execl fail");
            exit(EXIT_FAILURE);
        }
        cores[i].pid = pid;
        close(cores[i].main_to_core[0]);
        close(cores[i].core_to_main[1]);
        cores[i].status = 0; 
    }

    // log main process start
    printf("main process started with PID: %d\n", getpid());
    fflush(stdout);
    int current_task = 1;
    int tasks_assigned = 0;

    // assign and collect tasks/results
    while (tasks_assigned < total_tasks || cores[0].status == 1 || cores[1].status == 1 || cores[2].status == 1 || msg_num_core[0] > 0 || msg_num_core[1] > 0 || msg_num_core[2] > 0) {
        for (int i = 0; i < NUM_CORES && tasks_assigned < total_tasks; i++) {
            if (msg_num_core[i] == 0 && cores[i].status == 0) { 
                ssize_t bytes_written = write(cores[i].main_to_core[1], &current_task, sizeof(current_task));
                if (bytes_written == -1) {
                    perror("write to main_to_core fail");
                } else {
                    printf("main process assigned task ID: %d to Core %d\n", current_task, i + 1);
                    fflush(stdout);
                    cores[i].status = 1; 

                    current_task++;
                    tasks_assigned++;
                }
            }
        }



        for (int i = 0; i < NUM_CORES; i++) {
            while (msg_num_core[i] > 0) {
                int result;
                ssize_t bytes_read = read(cores[i].core_to_main[0], &result, sizeof(result));
                if (bytes_read > 0) {
                    printf("Main process received result for task ID: %d from Core %d\n", result, i + 1);
                    fflush(stdout);
                    msg_num_core[i]--;         
                    cores[i].status = 0;      
                } else if (bytes_read == 0) {
                    break;
                } else {
                    if (errno == EINTR) {
                        continue; 
                    }
                    perror("read from core_to_main failed");
                    break;
                }
            }
        }

        usleep(100000); 
    }

    // collect results from cores based on received signals
    for (int i = 0; i < NUM_CORES; i++) {
        close(cores[i].main_to_core[1]); 
        close(cores[i].core_to_main[0]); 
    }
    for (int i = 0; i < NUM_CORES; i++) {
        waitpid(cores[i].pid, NULL, 0);
    }
    printf("All subprocesses have completed.\n");
    fflush(stdout);


    return 0;
}
