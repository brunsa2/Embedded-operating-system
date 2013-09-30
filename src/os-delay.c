#ifdef __AVR_TestEnv__
#include "../test/os-conf.h"
#include "../test/avr/io.h"
#else
#include "os-conf.h"
#include <avr/io.h>
#endif

#include "os-delay.h"
#include "os-core.h"

extern volatile t_process_control_block pcb[NUMBER_OF_PROCESSES];
extern uint32_t system_ticks;

/**
 * Delay task for specified number of ticks
 *
 * @param pid Process ID to delay
 * @param ticks Number of ticks to delay
 */
int8_t os_delay(uint8_t pid, uint32_t ticks) {
	if (pid >= NUMBER_OF_PROCESSES) {
		return -1;
	}
	uint32_t start_timestamp = system_ticks + ticks;
	ENTER_CRITICAL_SECTION();
	pcb[pid].start_timestamp = start_timestamp;
    pcb[pid].delayed = 1;
	LEAVE_CRITICAL_SECTION();
	os_switch_processes();
	return 0;
}

/**
 * Cancel any delay on a task
 * @param pid Process ID to cancel delay for
 */
int8_t os_cancel_delay(uint8_t pid) {
    if (pid < 0 || pid >= NUMBER_OF_PROCESSES) {
        return -1;
    }
    ENTER_CRITICAL_SECTION();
    pcb[pid].delayed = 0;
    LEAVE_CRITICAL_SECTION();
    os_switch_processes();
    return 0;
}