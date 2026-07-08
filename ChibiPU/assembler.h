// Copyright (c) 2026 yuumei-02. All Rights Reserved.
// See the LICENSE file for more information.

typedef struct {
   u8* memory;
   u32 memory_size;
   u32 write_head;
} Assembler;

void load_program_from_file(Assembler* self, cstr path, u32 offset);

void load_nn_instr(Assembler* self, InstrKind instr_kind);
void load_rr_instr(Assembler* self, InstrKind instr_kind, InstrRegister arg0, InstrRegister arg1);
void load_rv_instr(Assembler* self, InstrKind instr_kind, InstrRegister arg0, u32 arg1);
void load_rn_instr(Assembler* self, InstrKind instr_kind, InstrRegister arg0);
void load_vn_instr(Assembler* self, InstrKind instr_kind, u32 arg0);

