#include "SimImu.h"
#include "proba_sistema.h"

namespace Actors {
    void SimImu::behavior() {}

    DEFINE_PERIODIC_TASK(1024, SimImu, 10_ms, PRIO_SIMIMU)
}