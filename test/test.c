#include <stdio.h>

#include "CuTest.h"

CuSuite* CuGetSuite();

#ifdef __AVR_TestEnv__
int main(void) {
    CuString *output = CuStringNew();
    CuSuite *suite = CuSuiteNew();
    
    CuSuiteAddSuite(suite, CuGetSuite());
    
    CuSuiteRun(suite);
    CuSuiteSummary(suite, output);
    CuSuiteDetails(suite, output);
    printf("%s\n", output->buffer);
    return CuSuiteErrorCode(suite);
}
#endif