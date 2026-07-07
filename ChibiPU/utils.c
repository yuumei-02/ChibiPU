// Copyright (c) 2026 yuumei-02. All Rights Reserved.
// See the LICENSE file for more information.

#include <mcu/core.h>

#include "utils.h"

u32 set_nibble(u32 value, u32 from_bit, u32 nibble) {
   mcu_assert(nibble <= 0xF, "nibble must be less or equals to 0xF (U4_MAX)");
   mcu_assert(from_bit < 28, "from_bit must be less than 28");

   return (value & ~(0xF << from_bit)) | ((nibble & 0xF) << from_bit);
}

u32 read_nibble(u32 value, u32 from_bit) {
   mcu_assert(from_bit < 28, "from_bit must be less than 28");

   return (value >> from_bit) & 0xF;
}

