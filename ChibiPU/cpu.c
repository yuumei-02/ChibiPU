// Copyright (c) 2026 yuumei-02. All Rights Reserved.
// See the LICENSE file for more information.

#include <mcu/core.h>
#include <mcu/io.h>

#include <inttypes.h>

#include "utils.h"
#include "cpu.h"

const cstr Trap_to_cstr(Trap self) {
   switch (self) {
      case T_None:               return "None";
      case T_InvalidVariant:     return "Invalid instruction variant";
      case T_UnknownRegister:    return "Unknown register";
      case T_UnknownInstruction: return "Unknown instruction";
   }

   return "Unknown";
}

bool InstrRegister_is_valid(InstrRegister self) {
   switch (self) {
      case IR_RA0: return true;
      case IR_RA1: return true;
      case IR_RA2: return true;
      case IR_RA3: return true;
      case IR_RA4: return true;
      case IR_RA5: return true;
   }

   return false;
}

static inline Trap execute_mov(CPU* self, u8* main_memory, Instr* instruction) {
   InstrVariant variant = read_nibble(instruction->variant, 0);
   InstrRegister dest_reg = read_nibble(instruction->variant, 4);

   switch (variant) {
      case IV_NN: return T_InvalidVariant;

      case IV_RR: {
         InstrRegister src_reg =  read_nibble(instruction->variant, 8);
         if (!InstrRegister_is_valid(dest_reg) || !InstrRegister_is_valid(src_reg))
            return T_UnknownRegister;

         self->argument_registers[dest_reg] = self->argument_registers[src_reg];
         self->process_registers.rip += 4;
      } return T_None;

      case IV_RV: {
         if (!InstrRegister_is_valid(dest_reg))
            return T_UnknownRegister;
         u32 value = *(u32*) (main_memory + self->process_registers.rip + sizeof(u32));

         self->argument_registers[dest_reg] = value;
         self->process_registers.rip += 8;
      } return T_None;
   }

   panic("unreachable");
}

bool CPU_execute_next(CPU* self, void* main_memory) {
   mcu_assert(self != nullptr, "self can't be null");
   mcu_assert(main_memory != nullptr, "main_memory can't be null");

   Instr* instruction = (Instr*) ((u8*) main_memory + self->process_registers.rip);
   Trap result;

   switch (instruction->kind) {
      case IK_Halt: return true;
      case IK_Mov:  result = execute_mov(self, main_memory, instruction); goto handle_result;
   }

   result = T_UnknownInstruction;

handle_result:
   if (result != T_None) {
      eprintln("[!] CPU trap triggered. \"%s\"", Trap_to_cstr(result));
      return true;
   }

   return false;
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

