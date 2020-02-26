#include <stdio.h>
#include "spacecat_vm.h"
#define HEX_MAX_SIZE 2

int main() {
    run("202F21015F10C000");
    printf("%c", return_stdout());
    return 0;
}