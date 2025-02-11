#include <stdio.h>
#include <NDL.h>

int main() {
    uint32_t now, lst;
    lst = NDL_GetTicks();
    while (1) {
        now = NDL_GetTicks();
        if (now - lst >= 500) {
            printf("Tick-tock...\n");
            lst = now;
        }
    }
    return 0;
}
