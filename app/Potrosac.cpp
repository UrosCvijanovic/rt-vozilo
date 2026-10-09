// app/Potrosac.cpp -- kod nivoa detalja za aktera Potrosac (model/proba.room).
// enactment = ports, ports = or: behavior() se poziva kad je u aktivaciji procitana poruka.
#include <cstdint>
#include "stm32f405xx.h"   // TIM2
#include "Potrosac.h"
#include "proba.h"

volatile uint32_t pingsReceived = 0;   // obradjene poruke
volatile uint32_t lastPing      = 0;   // vreme slanja poslednje obradjene poruke
volatile uint32_t lastR         = 0;   // R poslednje poruke: od slanja do obrade, us
volatile uint32_t maxR          = 0;
volatile uint32_t maxPerAct     = 0;   // najvise poruka obradjenih u jednoj aktivaciji

namespace Actors {

    void Potrosac::behavior () {
        uint32_t uOvojAktivaciji = 0;
        for (;;) {
            // Obrada jedne poruke, do kraja (run-to-completion po poruci).
            const auto* m = ulaz.msg();
            if (m && m->signal == PingProtocol::Signal::ping) {
                pingsReceived = pingsReceived + 1;
                uOvojAktivaciji = uOvojAktivaciji + 1;
                lastPing = m->ping;
                const uint32_t R = TIM2->CNT - m->ping;
                lastR = R;
                if (R > maxR) maxR = R;
            }
            if (!DRAIN) break;

            // Praznjenje reda: jezgro pamti najvise jedan dodatni signal (zastavica
            // dogadjaja), a biblioteka u aktivaciji cita jednu poruku po portu --
            // bez ovoga poruke mogu ostati zaglavljene u redu. Citanje pod mutexom
            // aktera, kao u Actor::process().
            lock();
            const bool jos = ulaz.read();
            unlock();
            if (!jos) break;
        }
        if (uOvojAktivaciji > maxPerAct) maxPerAct = uOvojAktivaciji;
    }

    // stek 1024 B
    DEFINE_SPORADIC_TASK(1024, Potrosac, PRIO_S)

}