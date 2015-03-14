// Copyright (C) 2015 Stojan Dimitrovski
//
// This file is distributed under The MIT License. Please see the enclosed file
// LICENSE.txt for the full text.

#pragma once

#include <cstddef>

namespace cu {
namespace IO {
namespace Endianness {

  inline bool IsBigEndian() {
    #ifndef CU_IO_ASSUME_ENDIANNESS
    static const int endianness_check = 1;

    char* end = (char*) &endianness_check;

    return end[0] == 0;
    #else
    return CU_IO_ASSUME_ENDIANNESS_IS_BIG_ENDIAN;
    #endif
  }

  template <typename T>
  inline T Reverse(T& value) {
    char* a = (char*) &value;
    char out[sizeof(T)];

    for (std::size_t i = 0; i < sizeof(T); i++) {
      out[i] = a[sizeof(T) - i - 1];
    }

    T b = *((T*) out);

    return b;
  }

  template <typename T>
  inline T MachineToBigEndian(T& value) {
    if (!IsBigEndian()) {
      return Reverse(value);
    }

    return value;
  }

  template <typename T>
  inline T MachineToLittleEndian(T& value) {
    if (IsBigEndian()) {
      return Reverse(value);
    }

    return value;
  }

} // BigEndian
} // IO
} // cu
