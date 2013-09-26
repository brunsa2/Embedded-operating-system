#include <string.h>

#include "CuTest.h"
#include "../src/os-core.h"

extern volatile t_process_control_block pcb[NUMBER_OF_PROCESSES];
extern volatile unsigned long int stack_high, stack_low;
extern volatile uint8_t idle_task_stack[IDLE_TASK_STACK_SIZE] NOINIT;
extern volatile uint16_t quantum_ticks;
extern volatile uint32_t system_ticks;
extern volatile uint8_t current_process;

static void test_os_init(CuTest *test) {
    stack_high = 0x12;
    stack_low = 0x34;
    os_init();
    CuAssertUint8Equals(test, 1, pcb[0].running);
    CuAssertUint8Equals(test, 0, pcb[0].delayed);
    CuAssertUint8Equals(test, 0, pcb[0].suspended);
    CuAssertUint8Equals(test, 0, pcb[0].semaphore_blocked);
    CuAssertUint32Equals(test, 0, pcb[0].start_timestamp);
    CuAssertUint16Equals(test, 0x1234, (unsigned long int) pcb[0].stack_pointer);
    CuAssertUint8Equals(test, 1, pcb[1].running);
    CuAssertUint8Equals(test, 0, pcb[1].delayed);
    CuAssertUint8Equals(test, 0, pcb[1].suspended);
    CuAssertUint8Equals(test, 0, pcb[1].semaphore_blocked);
    CuAssertUint32Equals(test, 0, pcb[1].start_timestamp);
    CuAssertLongEquals(test, (long) &idle_task_stack[IDLE_TASK_STACK_SIZE - 1] - sizeof(t_task_stack_frame), (long) pcb[1].stack_pointer);
}

static void test_timer_tick(CuTest *test) {
    quantum_ticks = 0;
    system_ticks = 0;
    
    timer_interrupt();
    
    CuAssertUint16Equals(test, 1, quantum_ticks);
    CuAssertUint32Equals(test, 1, system_ticks);
}

static void test_timer_tick_at_quantum_end(CuTest *test) {
    quantum_ticks = 9;
    system_ticks = 9;
    
    timer_interrupt();
    
    CuAssertUint16Equals(test, 0, quantum_ticks);
    CuAssertUint32Equals(test, 10, system_ticks);
}

static void test_timer_tick_at_system_tick_rollover(CuTest *test) {
    quantum_ticks = 9;
    system_ticks = 14;
    memset((void *) pcb, 0, NUMBER_OF_PROCESSES * sizeof(t_process_control_block));
    pcb[0].delayed = 1;
    pcb[0].start_timestamp = 20;
    
    timer_interrupt();
    
    CuAssertUint16Equals(test, 0, quantum_ticks);
    CuAssertUint32Equals(test, 0, system_ticks);
    CuAssertUint32Equals(test, 4, pcb[0].start_timestamp);
}

static void test_timer_tick_at_system_tick_rollover_after_delay(CuTest *test) {
    quantum_ticks = 9;
    system_ticks = 16;
    memset((void *) pcb, 0, NUMBER_OF_PROCESSES * sizeof(t_process_control_block));
    pcb[0].delayed = 1;
    pcb[0].start_timestamp = 20;
    
    timer_interrupt();
    
    CuAssertUint16Equals(test, 0, quantum_ticks);
    CuAssertUint32Equals(test, 0, system_ticks);
    CuAssertUint32Equals(test, 2, pcb[0].start_timestamp);
}

static void test_get_current_pid(CuTest *test) {
    current_process = 0x12;
    
    uint8_t current_process_from_function = os_get_current_pid();
    
    CuAssertUint8Equals(test, current_process, current_process_from_function);
}

CuSuite* CuGetSuite(void) {
    CuSuite *suite = CuSuiteNew();
    
    SUITE_ADD_TEST(suite, test_os_init);
    SUITE_ADD_TEST(suite, test_timer_tick);
    SUITE_ADD_TEST(suite, test_timer_tick_at_quantum_end);
    SUITE_ADD_TEST(suite, test_timer_tick_at_system_tick_rollover);
    SUITE_ADD_TEST(suite, test_timer_tick_at_system_tick_rollover_after_delay);
    SUITE_ADD_TEST(suite, test_get_current_pid);
        
    return suite;
}