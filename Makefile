# ============================================================
#  rt-vozilo (build za STM32F405 (Cortex-M4F) + HDS RTOS)
#  Pokretanje u QEMU (netduinoplus2), izlaz preko semihostinga
# ============================================================
SHELL := /bin/bash
# ---- Alati ----
PREFIX  := arm-none-eabi-
CXX     := $(PREFIX)g++
CC 		:= $(PREFIX)gcc
SIZE    := $(PREFIX)size
OBJCOPY := $(PREFIX)objcopy

# ---- Direktorijumi ----
RTOS_DIR := rtos/RTOS
BUILD    := build
TARGET   := $(BUILD)/app

# ---- Fajlovi iz CubeIDE (kopirati u koren projekta) ----
STARTUP   := startup_stm32f405xx.s
LDSCRIPT  := STM32F405RGTx_FLASH.ld


# ---- Direktorijumi RTOS-a (bez iar) ----
RTOS_SUBDIRS := $(RTOS_DIR) \
                $(RTOS_DIR)/api \
                $(RTOS_DIR)/api_imp \
                $(RTOS_DIR)/common \
                $(RTOS_DIR)/compiler/gcc \
                $(RTOS_DIR)/core \
                $(RTOS_DIR)/hal \
                $(RTOS_DIR)/mpu \
                $(RTOS_DIR)/sync \
                $(RTOS_DIR)/timing
				
# ---- Izvorni fajlovi ----
# Sve iz RTOS-a osim IAR grane
RTOS_CPP := $(foreach d,$(RTOS_SUBDIRS),$(wildcard $(d)/*.cpp))

# GCC asemblerski fajlovi navedeni eksplicitno (Windows ne razlikuje .S i .s)
RTOS_ASM := $(RTOS_DIR)/compiler/gcc/hal_handlers_gcc.S \
            $(RTOS_DIR)/compiler/gcc/prep_first_task_gcc.S

# Tvoj kod
APP_CPP  := $(wildcard app/*.cpp) $(wildcard sim/*.cpp)
MAIN_CPP := main.cpp

ROOM_CPP := $(wildcard ROOM/*.cpp) $(wildcard actors/*.cpp) $(wildcard protocols/*.cpp)

SOURCES_CPP := $(RTOS_CPP) $(APP_CPP) $(MAIN_CPP) $(ROOM_CPP)

SOURCES_C := system_stm32f4xx.c

OBJECTS := $(addprefix $(BUILD)/,$(SOURCES_CPP:.cpp=.o)) \
           $(addprefix $(BUILD)/,$(SOURCES_C:.c=.o)) \
           $(addprefix $(BUILD)/,$(RTOS_ASM:.S=.o)) \
           $(addprefix $(BUILD)/,$(STARTUP:.s=.o))

# ---- Include putanje: svi direktorijumi RTOS-a osim IAR ----
INCLUDES := $(addprefix -I,$(RTOS_SUBDIRS)) \
            -I$(RTOS_DIR)/linker \
            -ICMSIS/Include \
            -ICMSIS/Device/ST/STM32F4xx/Include \
            -Iapp -Isim -I. \
            -IROOM -Iactors -Iprotocols -Icompat

# ---- Flagovi ----
CPU := -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard


DEFS := -DSTM32F405xx -DQEMU_SYSCLK_HZ=168000000

# -O2 je obavezno: bez optimizacije vremena izvrsavanja (C) nisu realna
# -fno-exceptions/-fno-rtti: standard za embedded C++
CXXFLAGS := $(CPU) -std=c++17 -O2 -g3 \
            -fno-exceptions -fno-rtti -fno-threadsafe-statics \
            -ffunction-sections -fdata-sections \
            -Wall -Wextra -Wno-unused-parameter \
            -Wno-reorder -Wno-parentheses -Wno-unused-function \
            $(DEFS) $(INCLUDES)

ASFLAGS := $(CPU) -g3 -x assembler-with-cpp $(DEFS) $(INCLUDES)

# --specs=rdimon.specs: semihosting (printf ide u konzolu QEMU-a)
LDFLAGS  := $(CPU) -T$(LDSCRIPT) -L$(RTOS_DIR)/linker \
            -Wl,--gc-sections -Wl,-Map=$(BUILD)/app.map \
            --specs=rdimon.specs -lrdimon

# ---- QEMU ----
QEMU     := qemu-system-arm
MACHINE  := netduinoplus2
# icount vezuje virtuelno vreme za broj instrukcija -> ponovljiva merenja
ICOUNT   := -icount shift=3,align=off,sleep=off
QEMUFLAGS := -machine $(MACHINE) -cpu cortex-m4 -nographic -semihosting-config enable=on,target=native

CFLAGS := $(CPU) -std=c11 -O2 -g3 -ffunction-sections -fdata-sections \
          $(DEFS) $(INCLUDES)

# ============================================================

.PHONY: all clean run trace debug info

all: $(TARGET).elf

$(TARGET).elf: $(OBJECTS) $(LDSCRIPT)
	@mkdir -p $(dir $@)
	$(CXX) $(OBJECTS) $(LDFLAGS) -o $@
	@echo "-------------------------------------------"
	@$(SIZE) $@

$(BUILD)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/%.o: %.S
	@mkdir -p $(dir $@)
	$(CXX) $(ASFLAGS) -c $< -o $@

$(BUILD)/%.o: %.s
	@mkdir -p $(dir $@)
	$(CXX) $(ASFLAGS) -c $< -o $@


# Obicno pokretanje (prekid: Ctrl+A pa X)
run: $(TARGET).elf
	$(QEMU) $(QEMUFLAGS) -kernel $<

# Pokretanje sa determinističkim vremenom, izlaz u fajl
trace: $(TARGET).elf
	$(QEMU) $(QEMUFLAGS) $(ICOUNT) -kernel $< > trace.csv

# Debug: u drugom terminalu -> arm-none-eabi-gdb build/app.elf
#                              target remote :1234
debug: $(TARGET).elf
	$(QEMU) $(QEMUFLAGS) -S -gdb tcp::1234 -kernel $<

clean:
	rm -rf $(BUILD) trace.csv

# Provera sta je uopste pokupljeno u build
info:
	@echo "RTOS cpp fajlova: $(words $(RTOS_CPP))"
	@echo "App cpp fajlova:  $(words $(APP_CPP))"
	@echo "--- Include putanje ---"
	@echo $(INCLUDES) | tr ' ' '\n'
