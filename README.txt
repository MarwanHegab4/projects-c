1. Project Requirements (60 points)
    a. Three core processes using `fork()` and `execl()`. 
        - In main.c, this is done in the for-loop: for (int i = 0; i < NUM_CORES; i++)

    b. Processes are reaped because they wait for all child processes to terminate using `waitpid()`.
        - In main.c, this is done using waitpid(cores[i].pid, NULL, 0)

    c. Two pipes per core are used to support two-way communication.
        - In main.c, this is done using pipe(cores[i].main_to_core) and pipe(cores[i].core_to_main)

    d. Unused pipe ends are closed in both parent and child processes.
        - In main.c (parent), this is done using close(cores[i].main_to_core[0]) and close(cores[i].core_to_main[1])
        - In core.c (child), this is done using close(cores[i].main_to_core[1]) and close(cores[i].core_to_main[0])

    e. Pipes are closed after all communication is done.
        - In main.c, this is done using close(cores[i].main_to_core[1]) and close(cores[i].core_to_main[0])
        - In core.c, this is done using close(read_fd) and close(write_fd)

    f. `write()` is used to send data through pipes.
        - In main.c, this is done using write(cores[i].main_to_core[1], &current_task, sizeof(current_task))
        - In core.c, this is done using write(write_fd, &task_id, sizeof(task_id))

    g. 'read()` is used to receive data from pipes.
        - In main.c, this is done using read(cores[i].core_to_main[0], &result, sizeof(result))
        - In core.c, this is done using read(read_fd, &task_id, sizeof(task_id))

    h. Signals are chosen according to register.
        - In signals.h, this is done by defining SIGNAL_CORE_1, SIGNAL_CORE_2, and SIGNAL_CORE_3

    i. Signal handlers are registered using `sigaction()`.
        - In main.c, this is done in lines 68 and the for-loop.

    j. The handler updates a `volatile sig_atomic_t` variable.
        - In main.c, this is done using handle_new_msg_from_core(int sig) and volatile sig_atomic_t

    k. Proper type of global variable is used as the indicator of new results
        - In main.c, this is done using volatile sig_atomic_t msg_num_core[NUM_CORES] 

    l. Cores send signals after processing a task.
        - In core.c, this is done using kill(getppid(), signal_to_send)

    m. Core subprocess  detect the close of pipes and quit waiting 
        - In core.c, this is done using if (bytes_read == 0) and the loop breaks.

    n. Error handling is implemented for all system calls.
        - In main.c and core.c, this is implemented by checking the return values of pipe(), fork(), read(), write(), and kill(). 

    o. The program accepts `<total_tasks>` and `<max_sleep_time>` as input arguments (this is done in main.c).

    p. The program avoids memory leakage.

2. 
    a. Functions and variables are named to reflect their purpose.
    b. Global variables are minimized and used appropriately.
    c. Control flow is used effectively to manage the program.
    d. The program avoids unnecessary blocking and confusing constructs.

3. 
    a. Throughout the proejct, I followed a consistent style to write the code.
    b. Indentation and spacing adhered to in the project.
    c. Variable names are used to describe what they hold.
    d. Comments are used throughout the project.
    e. Variables are documented in an understanble manner.
    f. Functions have comments to describe the parameters and return values.

4. 
    a. This document provides an onverview of the project.
    b. This document highlights how my code satisifies the requirments of the project.
    c. The code is compatible using Makefile so you can run it and test that it works.
