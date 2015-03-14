// Copyright (C) 2015 Stojan Dimitrovski
//
// This file is distributed under The MIT License. Please see the enclosed file
// LICENSE.txt for the full text.

#define CATCH_CONFIG_MAIN

#include <catch.hpp>

#include <cu/io/endianness.hpp>

TEST_CASE ("Should properly check if this machine is big-endian.", "[cu::IO::Endianness]") {
  REQUIRE (sizeof(int) >= 4);

  int check = 0x0A0B0C0D;

  char* checker = (char*) &check;

  if (checker[0] == 0x0A) {
    REQUIRE (cu::IO::Endianness::IsBigEndian() == true);
  } else {
    REQUIRE (cu::IO::Endianness::IsBigEndian() == false);
  }
}

TEST_CASE ("Should properly convert to big-endian and vice versa.", "[cu::IO::Endianness]") {
  int check = 0;

  if (cu::IO::Endianness::IsBigEndian()) {
    check = 0x0D0C0B0A;
  } else {
    check = 0x0A0B0C0D;
  }

  int converted = cu::IO::Endianness::Reverse<int> (check);

  volatile char* out = (char*) &converted;

  REQUIRE (out[0] == 0x0A);
  REQUIRE (out[1] == 0x0B);
  REQUIRE (out[2] == 0x0C);
  REQUIRE (out[3] == 0x0D);

  converted = cu::IO::Endianness::Reverse<int> (converted);

  REQUIRE (out[0] == 0x0D);
  REQUIRE (out[1] == 0x0C);
  REQUIRE (out[2] == 0x0B);
  REQUIRE (out[3] == 0x0A);
}
