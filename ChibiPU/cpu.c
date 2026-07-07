// Copyright (c) 2026 yuumei-02. All Rights Reserved.
// See the LICENSE file for more information.

#include <mcu/core.h>
#include <mcu/io.h>

#include <inttypes.h>

#include "utils.h"
#include "cpu.h"

static inline void execute_mov(CPU* self, u8* main_memory, Instr* instruction) {
   InstrVariant variant = read_nibble(instruction->variant, 0);

   switch (variant) {
      case IV_NN: mcu_todo("raise a trap");
      case IV_RR: mcu_todo("not yet implemented");
      case IV_RV: {
         // @todo: check if the dest register is valid and raise a trap if not
         InstrRegister dest_reg = read_nibble(instruction->variant, 4);
         u32 value = *(u32*) (main_memory + self->process_registers.rip + sizeof(u32));

         self->argument_registers[dest_reg] = value;
         self->process_registers.rip += 8;
      } return;
   }

   panic("unreachable");
}

bool CPU_execute_next(CPU* self, void* main_memory) {
   mcu_assert(self != nullptr, "self can't be null");
   mcu_assert(main_memory != nullptr, "main_memory can't be null");

   Instr* instruction = (Instr*) ((u8*) main_memory + self->process_registers.rip);

   switch (instruction->kind) {
      case IK_Halt: return true;
      case IK_Mov:  execute_mov(self, main_memory, instruction); return false;
   }

   panic("unreachable");
}

void CPU_debug_dump_registers(CPU* self) {
   mcu_assert(self != nullptr, "self can't be null");

   println("+----------------+-------------------------------+");
   println("| Process        | Argument                      |");
   println("+----------------+-------------------------------+");
   println("| [%08" PRIx32 "] RIP | [%08" PRIx32 "] RA0 [%08" PRIx32 "] RA3 |",
      self->process_registers.rip, self->argument_registers[0], self->argument_registers[3]);
   println("|                | [%08" PRIx32 "] RA1 [%08" PRIx32 "] RA4 |",
      self->argument_registers[1], self->argument_registers[4]);
   println("|                | [%08" PRIx32 "] RA2 [%08" PRIx32 "] RA5 |",
      self->argument_registers[2], self->argument_registers[5]);
   println("+----------------+-------------------------------+");
}

