#include <cstdio>

volatile int marker = 0;

void userMain(void) {
    marker = 42;
    while(true) { }
    // ovde se kasnije kreiraju zadaci
}
