#include <string.h>

#include "CuTest.h"
#include "../src/os-core.h"
#include "../src/os-semaphore.h"

extern volatile t_process_control_block pcb[NUMBER_OF_PROCESSES];
extern volatile uint8_t priority_buffer[NUMBER_OF_PROCESSES] NOINIT;
extern volatile uint8_t current_process;
extern volatile uint8_t nesting_level;

static void test_semaphore_init(CuTest *test) {
    t_os_semaphore semaphore;
    
    os_semaphore_init(&semaphore, 3);
    
    CuAssertUint8Equals(test, 3, semaphore.count);
    int pid;
    for (pid = 0; pid < NUMBER_OF_PROCESSES; pid++) {
        CuAssertUint8Equals(test, 0, semaphore.wait_list[pid]);
    }
}

static void test_semaphore_wait_when_ready(CuTest *test) {
    t_os_semaphore semaphore;
    os_semaphore_init(&semaphore, 1);
    
    os_semaphore_wait(&semaphore);
    
    CuAssertUint8Equals(test, 0, semaphore.count);
}

static void test_semaphore_wait_when_busy(CuTest *test) {
    t_os_semaphore semaphore;
    memset((void *) pcb, 0, NUMBER_OF_PROCESSES * sizeof(t_process_control_block));
    current_process = 0;
    os_semaphore_init(&semaphore, 1);
    semaphore.count = 0;
    
    os_semaphore_wait(&semaphore);
    
    CuAssertUint8Equals(test, 1, semaphore.wait_list[0]);
    CuAssertUint8Equals(test, 1, (uint8_t) pcb[0].semaphore_blocked);
    CuAssertUint8Equals(test, 0, semaphore.count);
}

static void test_semaphore_signal(CuTest *test) {
    t_os_semaphore semaphore;
    memset((void *) pcb, 0, NUMBER_OF_PROCESSES * sizeof(t_process_control_block));
    current_process = 0;
    pcb[0].semaphore_blocked = 1;
    pcb[1].semaphore_blocked = 1;
    os_semaphore_init(&semaphore, 1);
    semaphore.count = 0;
    semaphore.wait_list[0] = 1;
    
    os_semaphore_signal(&semaphore);
    
    CuAssertUint8Equals(test, 0, (uint8_t) pcb[0].semaphore_blocked);
    CuAssertUint8Equals(test, 1, (uint8_t) pcb[1].semaphore_blocked);
    CuAssertUint8Equals(test, 0, semaphore.wait_list[0]);
    CuAssertUint8Equals(test, 0, semaphore.count);
}

static void test_semaphore_signal_with_nothing_waiting(CuTest *test) {
    t_os_semaphore semaphore;
    memset((void *) pcb, 0, NUMBER_OF_PROCESSES * sizeof(t_process_control_block));
    current_process = 0;
    pcb[0].semaphore_blocked = 0;
    pcb[1].semaphore_blocked = 0;
    os_semaphore_init(&semaphore, 1);
    semaphore.count = 0;
    
    os_semaphore_signal(&semaphore);
    
    CuAssertUint8Equals(test, 1, semaphore.count);
}

static void test_semaphore_at_scheduler(CuTest *test) {
    current_process = 1;
    nesting_level = 0;
    memset((void *) pcb, 0, NUMBER_OF_PROCESSES * sizeof(t_process_control_block));
    memset((void *) priority_buffer, 0xff, NUMBER_OF_PROCESSES * sizeof(uint8_t));
    pcb[0].running = 1;
    pcb[1].running = 1;
    priority_buffer[0] = 0;
    priority_buffer[1] = 1;
    pcb[0].semaphore_blocked = 1;
    
    os_switch_processes();
    
    CuAssertUint8Equals(test, 1, current_process);
}

CuSuite* get_semaphore_suite(void) {
    CuSuite *suite = CuSuiteNew();
    
    SUITE_ADD_TEST(suite, test_semaphore_init);
    SUITE_ADD_TEST(suite, test_semaphore_wait_when_ready);
    SUITE_ADD_TEST(suite, test_semaphore_wait_when_busy);
    SUITE_ADD_TEST(suite, test_semaphore_signal);
    SUITE_ADD_TEST(suite, test_semaphore_signal_with_nothing_waiting);
    SUITE_ADD_TEST(suite, test_semaphore_at_scheduler);
    
    return suite;
}