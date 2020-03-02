#include <stdio.h>
#include "spacecat_vm_gk.h"
#include <time.h>
#define HEX_MAX_SIZE 2

int main() {
    time_t test_start_seconds;
    time_t benchmark_seconds;
    time(&test_start_seconds);
    run("2500207E21012220402F5221F20820FE5551F502C000");
    time(&benchmark_seconds);
    printf("First result: %f\n", difftime(benchmark_seconds, test_start_seconds));
    return 0;
}