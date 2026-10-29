// app/Plovilo.cpp -- ponasanje aktera Plovilo u probi A3 (model/sistem.room).
// Proba: komanda stize kroz dva relejna porta neostecena, port stanja drzi
// poslednju vrednost (zadrzavanje), a slanje ne budi periodicni task (4.1).
// Model kretanja (6.5 predaje) dolazi u fazi C.
#include <cstdint>
#include "stm32f405xx.h"   // TIM2
#include "Plovilo.h"
#include "proba3.h"

volatile MerenjePlovila mPlovilo{};

namespace Actors {

    // Stanje probe; u pravom Plovilu ovo ide u sekciju data aktera.
    static bool imaKomandu = false;
    static uint32_t poslednjeT = 0;   // vreme slanja poslednje vidjene komande

    void Plovilo::behavior () {
        const uint32_t sada = TIM2->CNT;
        if (mPlovilo.aktivacija == 0) mPlovilo.prvaUs = sada;
        if (mPlovilo.aktivacija == 1) mPlovilo.drugaUs = sada;
        ++mPlovilo.aktivacija;

        const auto* m = pogonIn.msg();
        if (!m || m->signal != KomandaPogona::Signal::pogon) {
            ++mPlovilo.bezKomande; // pre prve komande vazi T_L = T_R = 0 (3.4)
            return;
        }
        const PodaciPogona& p = m->pogon;

        // Nova poruka se prepoznaje samo po vremenu slanja (4.4).
        if (!imaKomandu || p.t != poslednjeT) {
            ++mPlovilo.nove;
            poslednjeT = p.t;
            imaKomandu = true;
        } else {
            ++mPlovilo.ponovljene;
        }

        const uint32_t starost = sada - p.t;
        if (starost > mPlovilo.maxStarostUs) mPlovilo.maxStarostUs = starost;

        if (p.tL + p.tR != 1.0f) ++mPlovilo.neispravne;
    }

    // stek 1024 B (float, 16.5), perioda 10 ms (tabela aktera)
    DEFINE_PERIODIC_TASK(1024, Plovilo, 10_ms, PRIO_PLOVILO)

}
