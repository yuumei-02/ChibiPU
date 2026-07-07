// Copyright (c) 2026 yuumei-02. All Rights Reserved.
// See the LICENSE file for more information.

#include <mcu/core.h>
#include <mcu/handlers.h>
#include <mcu/memory.h>

#include "utils.h"
#include "cpu.h"

#define MEMORY_SIZE (KiB * 4)
static u8 G_memory[MEMORY_SIZE] = {0};
static u32 G_program_head = 0;

Instr* get_program_head() {
   return (Instr*) (G_memory + G_program_head);
}

void advance_program_head(u32 slots) {
   G_program_head += sizeof(u32) * slots;
}

void push_halt() {
   *get_program_head() = (Instr) { .kind = IK_Halt };
   advance_program_head(1);
}

void push_mov_rv(InstrRegister dest_register, u32 value) {
   Instr* instr = get_program_head();
   instr->kind = IK_Mov;
   instr->variant = set_nibble(instr->variant, 0, IV_RV);
   instr->variant = set_nibble(instr->variant, 4, dest_register);
   advance_program_head(1);

   *((u32*) get_program_head()) = value;
   advance_program_head(1);
}

i32 main() {
   CPU cpu = {0};

   push_mov_rv(IR_RA0, 0xdfd3a1ff);
   push_mov_rv(IR_RA1, 0x181818ff);
   push_mov_rv(IR_RA5, 0x08080808);
   push_halt();

   while (!CPU_execute_next(&cpu, G_memory));
   CPU_debug_dump_registers(&cpu);

   return 0;
}

