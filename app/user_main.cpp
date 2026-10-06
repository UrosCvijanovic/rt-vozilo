#include <cstdio>
#include <cstdint>
#include "stm32f405xx.h"


#include "api/kernel.h"
#include "api/periodic_task.h"
#include "api/stack.h"

volatile int marker = 0;

// Deljene promenjive, samo za probu platforme
// U aplikaciji taskovi komuniciraju iskljucivo porukama (ROOM)
static volatile bool lowBusy = false; // L je usred duge petlje
static volatile uint32_t highRuns = 0; // ukupno aktivacija H
static volatile uint32_t highPreempts = 0; // aktivacije H dok je L bio usred petlje
static volatile uint32_t lowRuns = 0; // zavrsene aktivacije L

static Kernel::TaskTelemetry highTel{};
static Kernel::TaskTelemetry lowTel{};

__attribute__((noinline)) void probeDone() {
    marker = 1;
}

// H: kratak posao, period 100 ms, makeTaskCfg(3)
// Ako je H viseg prioriteta, prekida L usred petlje i highPreempts raste
class HighProbe : public Kernel::Runnable {
    public:
        void activate() override {
            highRuns = highRuns + 1;
            if (lowBusy) {
                highPreempts = highPreempts + 1;
            }
            if (highRuns == 50) { // ~5 s virtuelnog vremena
                probeDone();
            }
        }
};


// L: duga petlja (~0,8 s pod -icount shift=3), perioda 2000 ms, makeTaskCfg(5).
class LowProbe : public Kernel::Runnable {
public:
    void activate() override {
        lowBusy = true;
        for (volatile uint32_t i = 0; i < 15000000; i = i + 1) { }
        lowBusy = false;
        lowRuns = lowRuns + 1;
    }
};


void userMain(void) {
    // static: objekti moraju da nadzive userMain() i ne smeju na heap
    static HighProbe high;
    static Kernel::StackMem<256> highStack;
    static Kernel::PeriodicParams highParams;
    highParams.period = 100_ms;
    highParams.activation = 0_us;
    static Kernel::PeriodicTask highTask(high, &highStack, &highParams, Kernel::makeTaskCfg(3));
 
    static LowProbe low;
    static Kernel::StackMem<256> lowStack;
    static Kernel::PeriodicParams lowParams;
    lowParams.period = 2000_ms;
    lowParams.activation = 0_us;
    static Kernel::PeriodicTask lowTask(low, &lowStack, &lowParams, Kernel::makeTaskCfg(5));
    
    highTask.setTelemetry(&highTel);
    lowTask.setTelemetry(&lowTel);

    highTask.start();
    lowTask.start();
}
