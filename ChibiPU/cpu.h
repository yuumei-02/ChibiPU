// Copyright (c) 2026 yuumei-02. All Rights Reserved.
// See the LICENSE file for more information.

#pragma once

typedef struct {
   struct {
      u32 rip;
   } process_registers;

   u32 argument_registers[6];
} CPU;

typedef enum : u16 {
   IK_Halt,
   IK_Mov,
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

void CPU_debug_dump_registers(CPU* self);

