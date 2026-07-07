// Copyright (c) 2026 yuumei-02. All Rights Reserved.
// See the LICENSE file for more information.

#include <mcu/core.h>
#include <mcu/handlers.h>
#include <mcu/memory.h>

#include "utils.h"
#include "cpu.h"
#include "assembler.h"

#define MEMORY_SIZE (KiB * 4)
static u8 G_memory[MEMORY_SIZE] = {0};

i32 main() {
   CPU cpu = {0};
   Assembler assembler = {
      .memory = (u8*) G_memory,
      .memory_size = MEMORY_SIZE
   };

   load_instr_mov_rv(&assembler, IR_RA0, 0xdeadbeaf);
   load_instr_mov_rr(&assembler, IR_RA1, IR_RA0);
   load_instr_halt(&assembler);

   while (!CPU_execute_next(&cpu, G_memory));
   CPU_debug_dump_registers(&cpu);

   return 0;
}

