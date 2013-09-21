#include "CuTest.h"
#include "../src/os-core.h"

extern volatile t_process_control_block pcb[NUMBER_OF_PROCESSES];
extern volatile unsigned long int stack_high, stack_low;

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
}

CuSuite* CuGetSuite(void) {
    CuSuite *suite = CuSuiteNew();
    
    SUITE_ADD_TEST(suite, test_os_init);
        
    return suite;
}