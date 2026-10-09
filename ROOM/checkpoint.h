// Copyright 2026 by HDS OS d.o.o.

#ifndef room_checkpoint_h
#define room_checkpoint_h

namespace ROOM {

/// @brief Emits CP:<us>:<name>\r\n over the board UART (CM7 only when dual-core).
void checkpoint(const char* name);

/// @brief Emits CP:<us>:<name>:<detail>\r\n over the board UART.
void checkpoint(const char* name, unsigned detail);

} // namespace ROOM

#define CHECKPOINT(name) ROOM::checkpoint(name)
#define CHECKPOINT_DETAIL(name, detail) ROOM::checkpoint(name, detail)

#endif
