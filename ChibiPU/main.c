// Copyright (c) 2026 yuumei-02. All Rights Reserved.
// See the LICENSE file for more information.

#include <mcu/core.h>
#include <mcu/handlers.h>
#include <mcu/memory.h>

#include "cpu.h"

#define MEMORY_SIZE (KiB * 4)
static u8* G_memory[MEMORY_SIZE] = {0};
static u32 G_program_head = 0;

Instr* get_program_head() {
   Instr* ptr = (Instr*) (G_memory + G_program_head);
   G_program_head += sizeof(u32);

   return ptr;
}

void push_halt() {
   *get_program_head() = (Instr) { .kind = IK_Halt };
}

i32 main() {
   CPU cpu = {0};

   push_halt();
   while (!CPU_execute_next(&cpu, G_memory));

   cpu.argument_registers[3] = 0xdeadbeaf;
   CPU_debug_dump_registers(&cpu);

   return 0;
}

