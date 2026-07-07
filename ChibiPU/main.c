// Copyright (c) 2026 yuumei-02. All Rights Reserved.
// See the LICENSE file for more information.

#include <mcu/core.h>
#include <mcu/handlers.h>
#include <mcu/memory.h>

#include "utils.h"
#include "cpu.h"
#include "assembler.h"

#define MEMORY_SIZE (KiB * 4)
static u8 G_memory[MEMORY_SIZE] = {0};

i32 main() {
   CPU cpu = {0};
   Assembler assembler = {
      .memory = (u8*) G_memory,
      .memory_size = MEMORY_SIZE
   };

   load_rv_instr(&assembler, IK_Mov, IR_RA0, 0xdeadbeaf);
   load_rr_instr(&assembler, IK_Mov, IR_RA1, IR_RA0);

   load_rv_instr(&assembler, IK_Mov, IR_RA3, 1);
   load_rv_instr(&assembler, IK_Add, IR_RA3, 1);
   load_rr_instr(&assembler, IK_Add, IR_RA3, IR_RA3);
   load_rv_instr(&assembler, IK_Sub, IR_RA3, 2);
   load_rv_instr(&assembler, IK_Mul, IR_RA3, 4);
   load_rv_instr(&assembler, IK_Div, IR_RA3, 2);
   load_rr_instr(&assembler, IK_Sub, IR_RA3, IR_RA3);
   /* load_rr_instr(&assembler, IK_Div, IR_RA3, IR_RA3); */

   load_rv_instr(&assembler, IK_Test, IR_RA4, 0);

   load_nn_instr(&assembler, IK_Halt);

   while (!CPU_execute_next(&cpu, G_memory));
   CPU_debug_dump_registers(&cpu);

   return 0;
}

