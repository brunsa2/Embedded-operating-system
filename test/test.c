#include <stdio.h>

#include "CuTest.h"

CuSuite* get_core_suite();
CuSuite* get_delay_suite();
CuSuite* get_semaphore_suite();

#ifdef __AVR_TestEnv__
int main(void) {
    CuString *output = CuStringNew();
    CuSuite *suite = CuSuiteNew();
    
    CuSuiteAddSuite(suite, get_core_suite());
    CuSuiteAddSuite(suite, get_delay_suite());
    CuSuiteAddSuite(suite, get_semaphore_suite());
    
    CuSuiteRun(suite);
    CuSuiteSummary(suite, output);
    CuSuiteDetails(suite, output);
    printf("%s\n", output->buffer);
    return CuSuiteErrorCode(suite);
}
#endif

// TODO: Test result codes of functions