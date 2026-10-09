// app/Proizvodjac.cpp -- kod nivoa detalja za aktera Proizvodjac (model/proba.room).
#include <cstdint>
#include "stm32f405xx.h"   // TIM2
#include "Proizvodjac.h"
#include "proba.h"

volatile int marker = 0;
volatile uint32_t pJobs = 0;   // zavrseni poslovi proizvodjaca (broji se pre probeDone)
volatile uint32_t pingsSent = 0;
volatile uint32_t maxSendUs = 0;   // najduze trajanje jednog send(); u B = cena slanja

// noinline: function breakpoint staje samo kad je funkcija stvarno pozvana (14.6)
__attribute__((noinline)) void probeDone() {
    marker = 1;
}

// Kalibrisano opterecenje: 48 ns po iteraciji pod -icount shift=3 (15.3, 15.8)
static void burnUs(uint32_t us) {
    const uint32_t n = us * 125u / 6u;
    for (volatile uint32_t i = 0; i < n; i = i + 1) { }
}

namespace Actors {

    void Proizvodjac::behavior () {
        for (uint32_t k = 0; k < PINGS_PER_JOB; ++k) {
            const uint32_t t0 = TIM2->CNT;
            pingsSent = pingsSent + 1;
            izlaz.send(PingProtocol::ping(t0)); // poruka nosi vreme slanja
            const uint32_t d = TIM2->CNT - t0;
            if (d > maxSendUs) maxSendUs = d;
        }
        burnUs(1000);
        pJobs = pJobs + 1;
        if (pJobs == 10) { // t = 0,9 s
            probeDone();
        }
    }

    // stek 1024 B, perioda 100 ms
    DEFINE_PERIODIC_TASK(1024, Proizvodjac, 100_ms, PRIO_P)

}
