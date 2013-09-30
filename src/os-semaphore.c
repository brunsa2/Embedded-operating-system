#ifdef __AVR_TestEnv__
#include "../test/os-conf.h"
#include "../test/avr/io.h"
#else
#include "os-conf.h"
#include <avr/io.h>
#endif

#include "os-semaphore.h"
#include "os-core.h"

extern volatile t_process_control_block pcb[NUMBER_OF_PROCESSES];

void os_semaphore_init(t_os_semaphore *semaphore, uint8_t count) {
    semaphore->count = count;
    uint8_t pid;
    for (pid = 0; pid < NUMBER_OF_PROCESSES; pid++) {
        semaphore->wait_list[pid] = 0;
    }
}

int8_t os_semaphore_wait(t_os_semaphore *semaphore) {
    ENTER_CRITICAL_SECTION();
    if (semaphore->count > 0) {
        semaphore->count--;
        LEAVE_CRITICAL_SECTION();
        return 0;
    }
    uint8_t pid = os_get_current_pid();
    semaphore->wait_list[pid] = 1;
    pcb[pid].semaphore_blocked = 1;
    LEAVE_CRITICAL_SECTION();
    os_switch_processes();
    return 0;
}

int8_t os_semaphore_signal(t_os_semaphore *semaphore) {
    ENTER_CRITICAL_SECTION();
    uint8_t pid, task_is_waiting_for_semaphore = 0;
    for (pid = 0; pid < NUMBER_OF_PROCESSES; pid++) {
        if (semaphore->wait_list[pid]) {
            task_is_waiting_for_semaphore = 1;
            pcb[pid].semaphore_blocked = 0;
            semaphore->wait_list[pid] = 0;
            break;
        }
    }
    if (task_is_waiting_for_semaphore) {
        LEAVE_CRITICAL_SECTION();
        os_switch_processes();
    } else {
        semaphore->count++;
        LEAVE_CRITICAL_SECTION();
    }
    return 0;
}