// Copyright (c) 2026 yuumei-02. All Rights Reserved.
// See the LICENSE file for more information.

#pragma once

typedef enum {
   T_None,
   T_InvalidVariant,
   T_UnknownVariant,
   T_UnknownRegister,
   T_UnknownInstruction,
   T_DivisionByZero
} Trap;

typedef enum {
   RFL_Zero
} RFL_Flag;

typedef struct {
   struct {
      u32 rip;
      u32 rfl;
   } process_registers;

   u32 argument_registers[6];
} CPU;

typedef enum : u16 {
   IK_Halt,

   IK_Mov,

   IK_Add,
   IK_Sub,
   IK_Div,
   IK_Mul,

   IK_Test,
   IK_Jnz,
   IK_Jz,
} InstrKind;

// 0000 0000 0000 0000
//                ^ Arguments kind
//           ^ arg 1
//      ^ arg 2
// ^ arg 3

// [R] Register
// [V] Value
// [N] None
typedef enum : u16 {
   IV_NN,
   IV_RN,
   IV_VN,
   IV_RR,
   IV_RV,
} InstrVariant;

typedef enum : u16 {
   IR_RA0, IR_RA1,
   IR_RA2, IR_RA3,
   IR_RA4, IR_RA5
} InstrRegister;

typedef struct {
   InstrKind kind;
   u16 variant;
} Instr;

static_assert(sizeof(Instr) == sizeof(u32));

const cstr Trap_to_cstr(Trap self);
bool InstrRegister_is_valid(InstrRegister self);

/// Returns a boolean of whether or not to halt.
bool CPU_execute_next(CPU* self, void* main_memory);
void CPU_debug_dump_registers(CPU* self);

