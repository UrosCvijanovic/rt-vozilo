// app/Regulacija.cpp -- ponasanje aktera Regulacija u probi A3 (model/sistem.room).
// Proba: salje komandu pogona u svakoj periodi i meri cenu slanja kroz dva
// relejna porta (Upravljanje.pogonOut, Simulator.pogonIn) do periodicnog primaoca.
#include <cstdint>
#include "stm32f405xx.h"   // TIM2
#include "Regulacija.h"
#include "proba3.h"

volatile MerenjeRegulacije mRegulacija{};

namespace Actors {

    void Regulacija::behavior () {
        if (mRegulacija.aktivacija == 0) mRegulacija.prvaUs = TIM2->CNT;

        // tL u {0; 0,25; 0,5; 0,75}, tR = 1 - tL: oba su tacno predstavljiva u
        // float-u, pa Plovilo proverava tL + tR == 1 bez greske zaokruzivanja.
        const float tL = static_cast<float>(mRegulacija.aktivacija % 4u) * 0.25f;
        ++mRegulacija.aktivacija;

        const uint32_t t0 = TIM2->CNT;                     // vreme slanja (17.9)
        pogonOut.send(KomandaPogona::pogon(PodaciPogona{tL, 1.0f - tL, t0}));
        const uint32_t d = TIM2->CNT - t0;
        if (d > mRegulacija.maxSendUs) mRegulacija.maxSendUs = d;
        ++mRegulacija.poslato;
    }

    // stek 1024 B (float, 16.5), perioda 20 ms (tabela aktera)
    DEFINE_PERIODIC_TASK(1024, Regulacija, 20_ms, PRIO_REGULACIJA)

}
