// Copyright (c) 2026 yuumei-02. All Rights Reserved.
// See the LICENSE file for more information.

typedef struct {
   u8* memory;
   u32 memory_size;
   u32 write_head;
} Assembler;

void load_program_from_file(const cstr path, void* memory, u32 memory_size, u32 offset);

void load_instr_halt(Assembler* self);
void load_instr_mov_rv(Assembler* self, InstrRegister dest_register, u32 value);
void load_instr_mov_rr(Assembler* self, InstrRegister dest_register, u32 src_register);

