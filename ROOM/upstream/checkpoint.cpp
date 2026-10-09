// Copyright 2026 by HDS OS d.o.o.

#include "checkpoint.h"

#include "../kernel/core/cpu_local.h"
#include "../tests/board/board.h"
#include "api/sync.h"

namespace ROOM {

namespace {

Kernel::Mutex checkpointMutex;

void write_char(char c)
{
    char buf[2] = {c, '\0'};
    board_uart_write(buf);
}

void write_dec_u64(uint64_t value)
{
    char tmp[21];
    unsigned idx = 0;
    if (value == 0U) {
        write_char('0');
        return;
    }
    while (value > 0U && idx < sizeof(tmp)) {
        tmp[idx++] = static_cast<char>('0' + (value % 10U));
        value /= 10U;
    }
    while (idx > 0U) {
        write_char(tmp[--idx]);
    }
}

void write_dec_u32(unsigned value)
{
    write_dec_u64(static_cast<uint64_t>(value));
}

void write_text(const char* text)
{
    if (text == nullptr) {
        return;
    }
    board_uart_write(text);
}

void emit_line(const char* name, unsigned detail, bool hasDetail)
{
    if (name == nullptr) {
        return;
    }

    const uint64_t us = static_cast<uint64_t>(Core::CPULocal::getTiming()->getTime());

    checkpointMutex.acquire();

    write_text("CP:");
    write_dec_u64(us);
    write_char(':');
    write_text(name);
    if (hasDetail) {
        write_char(':');
        write_dec_u32(detail);
    }
    write_text("\r\n");

    checkpointMutex.release();
}

} // namespace

void checkpoint(const char* name)
{
    emit_line(name, 0U, false);
}

void checkpoint(const char* name, unsigned detail)
{
    emit_line(name, detail, true);
}

} // namespace ROOM
