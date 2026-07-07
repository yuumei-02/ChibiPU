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

void load_nn_instr(Assembler* self, InstrKind instr_kind) {
   mcu_assert(self != nullptr, "self can't be null");
   mcu_assert((u64) self->write_head + 4 < self->memory_size, "OOM");

   *ptr_from_write_head(self) = (Instr) { .kind = instr_kind };
   advance_write_head(self, 1);
}

void load_rr_instr(Assembler* self, InstrKind instr_kind, InstrRegister arg0, InstrRegister arg1) {
   mcu_assert(self != nullptr, "self can't be null");
   mcu_assert((u64) self->write_head + 4 < self->memory_size, "OOM");

   Instr* instr = ptr_from_write_head(self);
   instr->kind = instr_kind;
   instr->variant = set_nibble(instr->variant, 0, IV_RR);
   instr->variant = set_nibble(instr->variant, 4, arg0);
   instr->variant = set_nibble(instr->variant, 8, arg1);
   advance_write_head(self, 1);
}

void load_rv_instr(Assembler* self, InstrKind instr_kind, InstrRegister arg0, u32 arg1) {
   mcu_assert(self != nullptr, "self can't be null");
   mcu_assert((u64) self->write_head + 8 < self->memory_size, "OOM");

   Instr* instr = ptr_from_write_head(self);
   instr->kind = instr_kind;
   instr->variant = set_nibble(instr->variant, 0, IV_RV);
   instr->variant = set_nibble(instr->variant, 4, arg0);
   advance_write_head(self, 1);

   *((u32*) ptr_from_write_head(self)) = arg1;
   advance_write_head(self, 1);
}

void load_rn_instr(Assembler* self, InstrKind instr_kind, InstrRegister arg0) {
   mcu_assert(self != nullptr, "self can't be null");
   mcu_assert((u64) self->write_head + 4 < self->memory_size, "OOM");

   Instr* instr = ptr_from_write_head(self);
   instr->kind = instr_kind;
   instr->variant = set_nibble(instr->variant, 0, IV_RN);
   instr->variant = set_nibble(instr->variant, 4, arg0);
   advance_write_head(self, 1);
}

void load_vn_instr(Assembler* self, InstrKind instr_kind, u32 arg0) {
   mcu_assert(self != nullptr, "self can't be null");
   mcu_assert((u64) self->write_head + 8 < self->memory_size, "OOM");

   Instr* instr = ptr_from_write_head(self);
   instr->kind = instr_kind;
   instr->variant = set_nibble(instr->variant, 0, IV_VN);
   advance_write_head(self, 1);

   *((u32*) ptr_from_write_head(self)) = arg0;
   advance_write_head(self, 1);
}

