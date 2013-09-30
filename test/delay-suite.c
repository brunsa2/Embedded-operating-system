#include <string.h>

#include "CuTest.h"
#include "../src/os-core.h"
#include "../src/os-delay.h"

extern volatile t_process_control_block pcb[NUMBER_OF_PROCESSES];
extern volatile uint8_t priority_buffer[NUMBER_OF_PROCESSES] NOINIT;
extern volatile unsigned long int stack_high, stack_low;
extern volatile uint8_t idle_task_stack[IDLE_TASK_STACK_SIZE];
extern volatile uint16_t quantum_ticks;
extern volatile uint32_t system_ticks;
extern volatile uint8_t current_process;
extern volatile uint8_t nesting_level;

static void test_delay(CuTest *test) {
    memset((void *) pcb, 0, NUMBER_OF_PROCESSES * sizeof(t_process_control_block));
    system_ticks = 1000;
    
    os_delay(0, 1000);
    
    CuAssertUint8Equals(test, 1, (uint8_t) pcb[0].delayed);
    CuAssertUint32Equals(test, 2000, pcb[0].start_timestamp);
}

static void test_cancel_delay(CuTest *test) {
    memset((void *) pcb, 0, NUMBER_OF_PROCESSES * sizeof(t_process_control_block));
    pcb[0].delayed = 1;
    
    os_cancel_delay(0);
    
    CuAssertUint8Equals(test, 0, (uint8_t) pcb[0].delayed);
}

static void test_delay_at_scheduler(CuTest *test) {
    current_process = 1;
    system_ticks = 0;
    nesting_level = 0;
    memset((void *) pcb, 0, NUMBER_OF_PROCESSES * sizeof(t_process_control_block));
    memset((void *) priority_buffer, 0xff, NUMBER_OF_PROCESSES * sizeof(uint8_t));
    pcb[0].running = 1;
    pcb[1].running = 1;
    priority_buffer[0] = 0;
    priority_buffer[1] = 1;
    pcb[0].delayed = 1;
    pcb[0].start_timestamp = 10;
    
    os_switch_processes();
    
    CuAssertUint8Equals(test, 1, current_process);
}

CuSuite* get_delay_suite(void) {
    CuSuite *suite = CuSuiteNew();
    
    SUITE_ADD_TEST(suite, test_delay);
    SUITE_ADD_TEST(suite, test_cancel_delay);
    SUITE_ADD_TEST(suite, test_delay_at_scheduler);
    
    return suite;
}