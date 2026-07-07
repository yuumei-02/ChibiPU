// Copyright (c) 2026 yuumei-02. All Rights Reserved.
// See the LICENSE file for more information.

typedef struct {
   u8* memory;
   u32 memory_size;
   u32 write_head;
} Assembler;

void load_program_from_file(const cstr path, void* memory, u32 memory_size, u32 offset);

void load_nn_instr(Assembler* self, InstrKind instr_kind);
void load_rr_instr(Assembler* self, InstrKind instr_kind, InstrRegister dest_register, InstrRegister src_register);
void load_rv_instr(Assembler* self, InstrKind instr_kind, InstrRegister dest_register, u32 value);

