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

i32 main(i32 argc, cstr argv[]) {
   CPU cpu = {0};
   Assembler assembler = {
      .memory = (u8*) G_memory,
      .memory_size = MEMORY_SIZE
   };

   if (argc > 1) {
      if (load_program_from_file(&assembler, argv[1], 0))
         return 1;
   }

   while (!CPU_execute_next(&cpu, G_memory));
   CPU_debug_dump_registers(&cpu);

   return 0;
}

