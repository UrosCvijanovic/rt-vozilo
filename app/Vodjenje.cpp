#include "Vodjenje.h"
#include "proba_sistema.h"

namespace Actors {
    void Vodjenje::behavior() {}

    DEFINE_PERIODIC_TASK(1024, Vodjenje, 100_ms, PRIO_VODJENJE)
}