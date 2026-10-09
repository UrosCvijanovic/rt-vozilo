// app/main.cpp -- userMain(): pravi i pokrece vrsnog aktera (dokumentacija alata).
// Poziva ga jezgro iz InitializerTask-a, privilegovano, na steku od 512 B (16.3).
#include <cstdio>
#include "ROOM/lib.h"
#include "api/kernel.h"
#include "Proba_base.h"
#include "Proizvodjac.h"
#include "Potrosac.h"

Kernel::TaskTelemetry proizvodjacTel{};
Kernel::TaskTelemetry potrosacTel{};

void userMain() {
    printf("userMain: start\n");

    // Telemetrija ide kroz TaskConfig aktera, pa mora PRE build() (tada se prave taskovi).
    Actors::Proizvodjac::setTaskTelemetry(&proizvodjacTel);
    Actors::Potrosac::setTaskTelemetry(&potrosacTel);

    // Isto sto i makro START iz ROOM/lib.h, raspisano: makro se ne prevodi GCC-om.
    // Proba nema ponasanje, pa je vrsni akter generisana klasa Proba_base.
    resetSharedBuffersAllocator();
    ROOM::Actor* root = Actors::Proba_base::create(nullcoreid, nullcoreid);
    root->build(nullcoreid);     // baferi portova, taskovi, mutexi aktera
    root->start();
}