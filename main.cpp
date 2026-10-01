#include <cstdio>
#include "stm32f405xx.h"

extern "C" void launchKernel(void);
extern "C" void initialise_monitor_handles(void);

int main() {
    // board-level init (takt, GPIO) — za sada nista
    initialise_monitor_handles();
    printf("main: start\n");
    launchKernel(); // ne vraca se
    while (true) {}
}
