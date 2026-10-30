// app/Misija.cpp -- ponasanje aktera Misija u probi A3 (model/sistem.room).
// enactment = ports, ports = or: behavior() se poziva kad je u aktivaciji
// procitana nova poruka. Proba: sporadicni primalac iza dva relejna porta se
// budi, ruta (140 B) stize neostecena, R po poruci (tacke 7 i 9).
// Masina stanja Misije dolazi posle dijagrama (17.1).
#include <cstdint>
#include "stm32f405xx.h"   // TIM2
#include "Misija.h"
#include "proba_sistema.h"

volatile MerenjeMisije mMisija{};

namespace Actors {

    // Obrada jedne poruke, do kraja (run-to-completion po poruci).
    static void obradi (const KomandaOperatera::Message& m) {
        uint32_t tSlanja = 0;
        switch (m.signal) {
            case KomandaOperatera::Signal::ruta: {
                ++mMisija.ruta;
                tSlanja = m.ruta.t;
                bool ok = (m.ruta.n == RUTA_N);
                for (uint8_t i = 0; ok && i < RUTA_N; ++i) {
                    ok = (m.ruta.wp[i].lat == RUTA_LAT0 + i) && (m.ruta.wp[i].lon == RUTA_LON0 + i);
                }
                if (!ok) ++mMisija.neispravnaRuta;
                break;
            }
            case KomandaOperatera::Signal::start:
                ++mMisija.start;
                tSlanja = m.start.t;
                break;
            case KomandaOperatera::Signal::stop:
                ++mMisija.stop;
                tSlanja = m.stop.t;
                break;
        }
        const uint32_t R = TIM2->CNT - tSlanja;
        if (R > mMisija.maxRUs) mMisija.maxRUs = R;
    }

    void Misija::behavior () {
        ++mMisija.aktivacija;
        uint32_t uOvojAktivaciji = 0;
        for (;;) {
            if (const auto* m = operaterIn.msg()) {
                obradi(*m);
                ++uOvojAktivaciji;
            }
            // Praznjenje reda (17.6, C2): jezgro pamti najvise jedan dodatni signal,
            // a process() cita jednu poruku po portu. Citanje pod mutexom aktera,
            // kao u Actor::process().
            lock();
            const bool jos = operaterIn.read();
            unlock();
            if (!jos) break;
        }
        if (uOvojAktivaciji > mMisija.maxPoAktivaciji) mMisija.maxPoAktivaciji = uOvojAktivaciji;
    }

    // stek 1024 B
    DEFINE_SPORADIC_TASK(1024, Misija, PRIO_MISIJA)

}
