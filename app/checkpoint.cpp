// app/checkpoint.cpp -- nasa implementacija ROOM::checkpoint (interfejs: ROOM/checkpoint.h).
// Originalna (ROOM/upstream/checkpoint.cpp) pise preko UART-a ploce i ukljucuje
// raspored HDS repozitorijuma ("../kernel/...", "../tests/board/board.h"); za QEMU pisemo svoju.
// Format je isti kao u originalu: CP:<us>:<ime>[:<detalj>], vreme iz TIM2 (isti sat kao telemetrija).
//
// Napomena: ispis ide preko semihostinga. Iz neprivilegovanih taskova to trazi
// -semihosting-config ...,userspace=on (16.5). Za sada se CHECKPOINT poziva samo
// pri pravljenju aktera, iz userMain (privilegovano).
#include <cstdio>
#include "stm32f405xx.h"
#include "checkpoint.h"

namespace ROOM {

    void checkpoint(const char* name) {
        printf("CP:%lu:%s\n", TIM2->CNT, name);
    }

    void checkpoint(const char* name, unsigned detail) {
        printf("CP:%lu:%s:%u\n", TIM2->CNT, name, detail);
    }

}
