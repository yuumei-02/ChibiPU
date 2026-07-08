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
      case T_UnknownVariant:     return "Unknown instruction variant";
      case T_UnknownRegister:    return "Unknown register";
      case T_UnknownInstruction: return "Unknown instruction";
      case T_DivisionByZero:     return "Division by zero";
      case T_OutOfBoundsAddress: return "Out of bounds address";
   }

   return "Unknown";
}

const cstr InstrKind_to_cstr(InstrKind self) {
   switch (self) {
      case IK_Halt:  return "Halt";
      case IK_Mov:   return "Mov";
      case IK_Load:  return "Load";
      case IK_Store: return "Store";
      case IK_Add:   return "Add";
      case IK_Sub:   return "Sub";
      case IK_Div:   return "Div";
      case IK_Mul:   return "Mul";
      case IK_Test:  return "Test";
      case IK_Jnz:   return "Jnz";
      case IK_Jz:    return "Jz";
   }

   return "Unknown";
}

const cstr InstrRegister_to_cstr(InstrRegister self) {
   switch (self) {
      case IR_RA0: return "RA0";
      case IR_RA1: return "RA1";
      case IR_RA2: return "RA2";
      case IR_RA3: return "RA3";
      case IR_RA4: return "RA4";
      case IR_RA5: return "RA5";
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

static Trap execute_load_store(CPU* self, u8* main_memory, u32 main_memory_size, Instr* instruction) {
   InstrVariant variant = read_nibble(instruction->variant, 0);
   InstrRegister dest_reg = read_nibble(instruction->variant, 4);

   switch (variant) {
      case IV_NN: [[fallthrough]];
      case IV_RN: [[fallthrough]];
      case IV_VN: return T_InvalidVariant;

      case IV_RR: {
         InstrRegister src_reg =  read_nibble(instruction->variant, 8);
         if (!InstrRegister_is_valid(dest_reg) || !InstrRegister_is_valid(src_reg))
            return T_UnknownRegister;

         switch (instruction->kind) {
            case IK_Mov: {
               self->argument_registers[dest_reg] = self->argument_registers[src_reg];
            } break;

            case IK_Load: {
               if (self->argument_registers[src_reg] >= main_memory_size)
                  return T_OutOfBoundsAddress;
               self->argument_registers[dest_reg] = *((u32*) (main_memory + self->argument_registers[src_reg]));
            } break;

            default: panic("unreachable");
         }

         self->process_registers.rip += 4;
      } return T_None;

      case IV_RV: {
         if (!InstrRegister_is_valid(dest_reg))
            return T_UnknownRegister;
         u32 value = *(u32*) (main_memory + self->process_registers.rip + sizeof(u32));

         switch (instruction->kind) {
            case IK_Mov: {
               self->argument_registers[dest_reg] = value;
            } break;

            case IK_Load: {
               if (value >= main_memory_size)
                  return T_OutOfBoundsAddress;
               self->argument_registers[dest_reg] = *((u32*) (main_memory + value));
            } break;

            default: panic("unreachable");
         }

         self->process_registers.rip += 8;
      } return T_None;
   }

   return T_UnknownVariant;
}

static Trap execute_arithmatic(CPU* self, u8* main_memory, Instr* instruction) {
   InstrVariant variant = read_nibble(instruction->variant, 0);
   InstrRegister dest_reg = read_nibble(instruction->variant, 4);

   switch (variant) {
      case IV_NN: [[fallthrough]];
      case IV_RN: [[fallthrough]];
      case IV_VN: return T_InvalidVariant;

      case IV_RR: {
         InstrRegister src_reg =  read_nibble(instruction->variant, 8);
         if (!InstrRegister_is_valid(dest_reg) || !InstrRegister_is_valid(src_reg))
            return T_UnknownRegister;

         switch (instruction->kind) {
            case IK_Add: self->argument_registers[dest_reg] += self->argument_registers[src_reg]; break;
            case IK_Sub: self->argument_registers[dest_reg] -= self->argument_registers[src_reg]; break;
            case IK_Mul: self->argument_registers[dest_reg] *= self->argument_registers[src_reg]; break;

            case IK_Div: {
               if (self->argument_registers[dest_reg] == 0 || self->argument_registers[src_reg] == 0)
                  return T_DivisionByZero;
               self->argument_registers[dest_reg] /= self->argument_registers[src_reg];
            } break;
            
            default: panic("unreachable");
         }

         self->process_registers.rip += 4;
      } return T_None;

      case IV_RV: {
         if (!InstrRegister_is_valid(dest_reg))
            return T_UnknownRegister;
         u32 value = *(u32*) (main_memory + self->process_registers.rip + sizeof(u32));

         switch (instruction->kind) {
            case IK_Add: self->argument_registers[dest_reg] += value; break;
            case IK_Sub: self->argument_registers[dest_reg] -= value; break;
            case IK_Mul: self->argument_registers[dest_reg] *= value; break;
            
            case IK_Div: {
               if (self->argument_registers[dest_reg] == 0 || value == 0)
                  return T_DivisionByZero;
               self->argument_registers[dest_reg] /= value;
            } break;
            
            default: panic("unreachable");
         }
         
         self->process_registers.rip += 8;
      } return T_None;
   }

   return T_UnknownVariant;
}

static Trap execute_test(CPU* self, u8* main_memory, Instr* instruction) {
   InstrVariant variant = read_nibble(instruction->variant, 0);
   InstrRegister dest_reg = read_nibble(instruction->variant, 4);

   switch (variant) {
      case IV_NN: [[fallthrough]];
      case IV_RN: [[fallthrough]];
      case IV_VN: return T_InvalidVariant;

      case IV_RR: {
         InstrRegister src_reg =  read_nibble(instruction->variant, 8);
         if (!InstrRegister_is_valid(dest_reg) || !InstrRegister_is_valid(src_reg))
            return T_UnknownRegister;

         if (self->argument_registers[dest_reg] == self->argument_registers[src_reg])
            self->process_registers.rfl |= (1 << RFL_Zero);
         
         self->process_registers.rip += 4;
      } return T_None;

      case IV_RV: {
         if (!InstrRegister_is_valid(dest_reg))
            return T_UnknownRegister;
         u32 value = *(u32*) (main_memory + self->process_registers.rip + sizeof(u32));

         if (self->argument_registers[dest_reg] == value)
            self->process_registers.rfl |= (1 << RFL_Zero);

         self->process_registers.rip += 8;
      } return T_None;
   }

   return T_UnknownVariant;
}

static Trap execute_jump(CPU* self, u8* main_memory, Instr* instruction) {
   InstrVariant variant = read_nibble(instruction->variant, 0);
   InstrRegister dest_reg = read_nibble(instruction->variant, 4);

   switch (variant) {
      case IV_NN: [[fallthrough]];
      case IV_RR: [[fallthrough]];
      case IV_RV: return T_InvalidVariant;

      case IV_RN: {
         if (!InstrRegister_is_valid(dest_reg))
            return T_UnknownRegister;

         bool zero_flag = ((self->process_registers.rfl >> RFL_Zero) & 1);

         switch (instruction->kind) {
            case IK_Jnz: {
               if (!zero_flag)
                  self->process_registers.rip = self->argument_registers[dest_reg];
               else
                  self->process_registers.rip += 4;
            } break;

            case IK_Jz: {
               if (zero_flag)
                  self->process_registers.rip = self->argument_registers[dest_reg];
               else
                  self->process_registers.rip += 4;
            } break;

            default: panic("unreachable");
         }
      } return T_None;

      case IV_VN: {
         u32 value = *(u32*) (main_memory + self->process_registers.rip + sizeof(u32));
         bool zero_flag = ((self->process_registers.rfl >> RFL_Zero) & 1);

         switch (instruction->kind) {
            case IK_Jnz: {
               if (!zero_flag)
                  self->process_registers.rip = value;
               else
                  self->process_registers.rip += 8;
            } break;

            case IK_Jz: {
               if (zero_flag)
                  self->process_registers.rip = value;
               else
                  self->process_registers.rip += 8;
            } break;

            default: panic("unreachable");
         }
      } return T_None;
   }

   return T_UnknownVariant;
}

bool CPU_execute_next(CPU* self, void* main_memory, u32 main_memory_size) {
   mcu_assert(self != nullptr, "self can't be null");
   mcu_assert(main_memory != nullptr, "main_memory can't be null");

   Instr* instruction = (Instr*) ((u8*) main_memory + self->process_registers.rip);
   Trap result;

   switch (instruction->kind) {
      case IK_Halt: return true;

      case IK_Store: mcu_todo("not yet implemented");
      case IK_Load:  [[fallthrough]];
      case IK_Mov:   result = execute_load_store(self, main_memory, main_memory_size, instruction); goto handle_result;

      case IK_Add: [[fallthrough]];
      case IK_Sub: [[fallthrough]];
      case IK_Mul: [[fallthrough]];
      case IK_Div: result = execute_arithmatic(self, main_memory, instruction); goto handle_result;

      case IK_Test: result = execute_test(self, main_memory, instruction); goto handle_result;

      case IK_Jnz: [[fallthrough]];
      case IK_Jz:  result = execute_jump(self, main_memory, instruction); goto handle_result;
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
   println("| [%08" PRIx32 "] RFL | [%08" PRIx32 "] RA1 [%08" PRIx32 "] RA4 |",
      self->process_registers.rfl, self->argument_registers[1], self->argument_registers[4]);
   println("|                | [%08" PRIx32 "] RA2 [%08" PRIx32 "] RA5 |",
      self->argument_registers[2], self->argument_registers[5]);
   println("+----------------+-------------------------------+");
}

