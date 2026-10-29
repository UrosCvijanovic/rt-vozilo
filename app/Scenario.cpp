// app/Scenario.cpp -- ponasanje aktera Scenario u probi A3 (model/sistem.room).
// Proba: po rasporedu iz proba3.h salje rutu, start i stop kroz dva relejna
// porta (Simulator.operaterOut, Upravljanje.operaterIn) sporadicnoj Misiji,
// i zavrsava probu u t = 1000 ms.
#include <cstdint>
#include "stm32f405xx.h"   // TIM2
#include "Scenario.h"
#include "proba3.h"

volatile int marker = 0;
volatile MerenjeScenarija mScenario{};

// noinline: function breakpoint staje samo kad je funkcija stvarno pozvana (14.6)
__attribute__((noinline)) void probeDone() {
    marker = 1;
}

namespace Actors {

    void Scenario::behavior () {
        const uint32_t a = mScenario.aktivacija;
        if (a == 0) mScenario.prvaUs = TIM2->CNT;
        ++mScenario.aktivacija;

        if (a == A_RUTA_START) {
            // Ruta pa start, istim portom: red cuva redosled (3.5).
            PodaciRute r{};
            for (uint8_t i = 0; i < RUTA_N; ++i) {
                r.wp[i].lat = RUTA_LAT0 + i;
                r.wp[i].lon = RUTA_LON0 + i;
            }
            r.n = RUTA_N;
            r.t = TIM2->CNT; // vreme slanja (17.9)
            operaterOut.send(KomandaOperatera::ruta(r));
            ++mScenario.poslato;

            operaterOut.send(KomandaOperatera::start(Vreme{TIM2->CNT}));
            ++mScenario.poslato;
        }

        if (a == A_STOP) {
            operaterOut.send(KomandaOperatera::stop(Vreme{TIM2->CNT}));
            ++mScenario.poslato;
        }

        if (a == A_KRAJ) {
            probeDone();
        }
    }

    // stek 1024 B, perioda 100 ms (tabela aktera)
    DEFINE_PERIODIC_TASK(1024, Scenario, 100_ms, PRIO_SCENARIO)

}
