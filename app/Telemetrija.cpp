#include "Telemetrija.h"
#include "proba_sistema.h"

namespace Actors {
    void Telemetrija::behavior() {}

    DEFINE_PERIODIC_TASK(1024, Telemetrija, 500_ms, PRIO_TELEMETRIJA)
}