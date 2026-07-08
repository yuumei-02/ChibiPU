// Copyright (c) 2026 yuumei-02. All Rights Reserved.
// See the LICENSE file for more information.

#pragma once

u32 set_nibble(u32 value, u32 from_bit, u32 nibble);
u32 read_nibble(u32 value, u32 from_bit);

void debug_print_memory_region(void* region, usize bytes);

