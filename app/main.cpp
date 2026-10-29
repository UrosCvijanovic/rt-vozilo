// app/main.cpp -- userMain(): pravi i pokrece vrsnog aktera Sistem (proba A3,
// model/sistem.room). Poziva ga jezgro iz InitializerTask-a, privilegovano,
// na steku od 512 B (16.3): samo pravljenje, ispis pre pokretanja i start().
#include <cstdio>
#include "ROOM/lib.h"
#include "api/kernel.h"
#include "actorConfig.h"         // poruke.h mora pre headera protokola (nemaju #include)
#include "KomandaPogona.h"
#include "KomandaOperatera.h"
#include "Sistem_base.h"
#include "Regulacija.h"
#include "Misija.h"
#include "Plovilo.h"
#include "Scenario.h"

// Telemetrija jezgra po tasku (15.7); iz nje koristimo samo min, max i activationsCount.
Kernel::TaskTelemetry regulacijaTel{};
Kernel::TaskTelemetry misijaTel{};
Kernel::TaskTelemetry ploviloTel{};
Kernel::TaskTelemetry scenarioTel{};

void userMain() {
    const uint32_t tim2 = TIM2->CNT;
    const uint32_t val  = SysTick->VAL;
    printf("TIM2 = %lu, SysTick VAL = %lu\n", tim2, val);

    printf("userMain: start\n");

    // Tacka 8: poruka = signal + najveci podatak iz union-a (ocekivano 16 i 140 B).
    printf("sizeof KomandaPogona::Message    = %u B\n", static_cast<unsigned>(sizeof(KomandaPogona::Message)));
    printf("sizeof KomandaOperatera::Message = %u B\n", static_cast<unsigned>(sizeof(KomandaOperatera::Message)));

    // Telemetrija ide kroz staticki TaskConfig aktera, pa mora PRE build() (17.4).
    Actors::Regulacija::setTaskTelemetry(&regulacijaTel);
    Actors::Misija::setTaskTelemetry(&misijaTel);
    Actors::Plovilo::setTaskTelemetry(&ploviloTel);
    Actors::Scenario::setTaskTelemetry(&scenarioTel);

    // Isto sto i makro START iz ROOM/lib.h, raspisano: makro se ne prevodi GCC-om (17.4).
    // Sistem nema ponasanje, pa je vrsni akter generisana klasa Sistem_base.
    resetSharedBuffersAllocator();
    ROOM::Actor* root = Actors::Sistem_base::create(nullcoreid, nullcoreid);   // CP: redosled pravljenja
    root->build(nullcoreid);     // baferi portova, taskovi, stekovi, mutexi aktera
    placementReport();           // stvarno zauzece regiona na ARM-u (dimenzionisanje, 18.2)
    root->start();
}