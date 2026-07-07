// Copyright (c) 2026 yuumei-02. All Rights Reserved.
// See the LICENSE file for more information.

#include <mcu/core.h>
#include <mcu/handlers.h>

#include "cpu.h"

i32 main() {
   CPU cpu = {0};

   cpu.argument_registers[3] = 0xdeadbeaf;
   CPU_debug_dump_registers(&cpu);

   return 0;
}

