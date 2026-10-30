#include "Fuzija.h"
#include "proba_sistema.h"

namespace Actors {
    void Fuzija::behavior() {}

    DEFINE_PERIODIC_TASK(1024, Fuzija, 10_ms, PRIO_FUZIJA)
}