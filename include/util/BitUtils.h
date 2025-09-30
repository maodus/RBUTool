#pragma once

static int short_bits[16] = {0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4};

inline int Count4Bit(char num) { return short_bits[num & 0xF]; }