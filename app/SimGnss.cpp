#include "SimGnss.h"
#include "proba_sistema.h"

namespace Actors {
    void SimGnss::behavior() {}

    DEFINE_PERIODIC_TASK(1024, SimGnss, 200_ms, PRIO_SIMGNSS)
}