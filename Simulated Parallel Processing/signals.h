#ifndef SIGNALS_H
#define SIGNALS_H
#include <signal.h>
#define SIGNAL_CORE_1 (SIGRTMIN)
#define SIGNAL_CORE_2 (SIGRTMIN + 1)
#define SIGNAL_CORE_3 (SIGRTMIN + 2)

void handle_new_msg_from_core(int sig);

#endif 

