#include <cstdio>
#include <cstdint>
#include "stm32f405xx.h"


#include "api/kernel.h"
#include "api/periodic_task.h"
#include "api/sporadic_task.h"
#include "api/stack.h"

// -------------------------------------------
// TEST: Sporadicni task S koji signalizira periodicni task P
// Pokretanje A: S viseg prioriteta od P, jedan signal po poslu P
// Pokretanje B; P viseg prioriteta od S, dva signala po poslu P

constexpr uint8_t PRIO_S = 3;
constexpr uint8_t PRIO_P = 2;
constexpr uint32_t SIGNALS_PERS_JOB = 2;



volatile int marker = 0;

// Deljene promenjive, samo za probu platforme
static volatile uint32_t pJobs = 0; // poslovi P (broji se pre probeDone)
static volatile uint32_t signalsSent = 0; // ukupno poslatih signala
static volatile uint32_t sActivations = 0; // ukupno aktivacija S
static volatile uint32_t lastSignalUs = 0;  // TIM2 neposredno pre poslednjeg signala

// Telemetrija (cfgUseTelemetry = 1)
static Kernel::TaskTelemetry pTel{};
static Kernel::TaskTelemetry sTel{};

static volatile uint32_t timer_value;

static Kernel::SporadicTask* consumerTask = nullptr;

__attribute__((noinline)) void probeDone() {
    marker = 1;
}

// Kalibrisano opterecenje: 48 ns po iteraciji pod -icount shift=3
static void burnUs(uint32_t us) {
    const uint32_t n = us * 125u / 6u;
    for (volatile uint32_t i = 0; i < n; i = i + 1) { }
}

// S: sporadicni potrosac, samo broji aktivacije
class ConsumerRunnable  : public Kernel::Runnable {
    public:
        void activate() override {
           sActivations = sActivations + 1;
        }
};


// L: duga petlja (~0,8 s pod -icount shift=3), perioda 2000 ms, makeTaskCfg(5).
class ProducerRunnable : public Kernel::Runnable {
public:
    void activate() override {
        for (uint32_t k = 0; k < SIGNALS_PERS_JOB; ++k) { 
            lastSignalUs = TIM2->CNT;
            signalsSent = signalsSent + 1;
            consumerTask->signal(1);
        }
        burnUs(1000);
        pJobs = pJobs + 1;
        if (pJobs == 10) { // posle ~1,8 s
            probeDone();
        }
    }
};


void userMain(void) {
    printf("userMain: start\n");
    printf("SystemCoreClock = %lu\n", SystemCoreClock);
    printf("SysTick CTRL = 0x%08lx, LOAD = %lu\n", SysTick->CTRL, SysTick->LOAD);
 
    static ConsumerRunnable consumer;
    static Kernel::StackMem<256> consumerStack;
    static Kernel::SporadicParams consumerParams;
    consumerParams.mask = 1;
    // consumerParams.mode = Kernel::SporadicParams::Or; // rezim: podrazumevani Or (dokumentacija)
    static Kernel::SporadicTask consumerT(consumer, &consumerStack, &consumerParams, Kernel::makeTaskCfg(PRIO_S));
    consumerTask = &consumerT;
 
    static ProducerRunnable producer;
    static Kernel::StackMem<256> producerStack;
    static Kernel::PeriodicParams producerParams;
    producerParams.period     = 200_ms;
    producerParams.activation = 0_us;
    static Kernel::PeriodicTask producerT(producer, &producerStack, &producerParams, Kernel::makeTaskCfg(PRIO_P));
 
    consumerT.setTelemetry(&sTel);
    producerT.setTelemetry(&pTel);
 
    consumerT.start();
    producerT.start();
}
