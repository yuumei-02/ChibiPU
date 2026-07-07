// Copyright (c) 2026 yuumei-02. All Rights Reserved.
// See the LICENSE file for more information.

#include <mcu/core.h>

#include "utils.h"
#include "cpu.h"
#include "assembler.h"

void load_program_from_file(const cstr path, void* memory, u32 memory_size, u32 offset) {
   unused path;
   unused memory;
   unused memory_size;
   unused offset;

   mcu_todo("not yet implemented");
}

static inline Instr* ptr_from_write_head(Assembler* assembler) {
   return (Instr*) (assembler->memory + assembler->write_head);
}

static inline void advance_write_head(Assembler* assembler, u32 slots) {
   assembler->write_head += sizeof(u32) * slots;
}

void load_instr_halt(Assembler* self) {
   mcu_assert(self != nullptr, "self can't be null");
   mcu_assert((u64) self->write_head + 4 < self->memory_size, "OOM");

   *ptr_from_write_head(self) = (Instr) { .kind = IK_Halt };
   advance_write_head(self, 1);
}

void load_instr_mov_rv(Assembler* self, InstrRegister dest_register, u32 value) {
   mcu_assert(self != nullptr, "self can't be null");
   mcu_assert((u64) self->write_head + 8 < self->memory_size, "OOM");

   Instr* instr = ptr_from_write_head(self);
   instr->kind = IK_Mov;
   instr->variant = set_nibble(instr->variant, 0, IV_RV);
   instr->variant = set_nibble(instr->variant, 4, dest_register);
   advance_write_head(self, 1);

   *((u32*) ptr_from_write_head(self)) = value;
   advance_write_head(self, 1);
}

void load_instr_mov_rr(Assembler* self, InstrRegister dest_register, u32 src_register) {
   mcu_assert(self != nullptr, "self can't be null");
   mcu_assert((u64) self->write_head + 4 < self->memory_size, "OOM");

   Instr* instr = ptr_from_write_head(self);
   instr->kind = IK_Mov;
   instr->variant = set_nibble(instr->variant, 0, IV_RR);
   instr->variant = set_nibble(instr->variant, 4, dest_register);
   instr->variant = set_nibble(instr->variant, 8, src_register);
   advance_write_head(self, 1);
}

