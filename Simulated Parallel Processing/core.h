#ifndef CORE_H
#define CORE_H
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <time.h>

void core_process(int core_id, int read_fd, int write_fd, int max_sleep_time);

#endif
