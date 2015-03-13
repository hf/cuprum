// Copyright (C) 2015 Stojan Dimitrovski
//
// This file is distributed under The MIT License. Please see the enclosed file
// LICENSE.txt for the full text.

#define CATCH_CONFIG_MAIN

#include <catch.hpp>

#include <cu/io/unifying_block_input_stream.hpp>

#include <cstddef>

using namespace std;

TEST_CASE ("UnifyingBlockInputStream should unify multiple data providers.", "[cu::IO::UnifyingBlockInputStream]") {
  vector<cu::IO::DataProvider*> providers;

  providers.push_back(new cu::IO::RawDataProvider(512, new char[512]));
  providers.push_back(new cu::IO::RawDataProvider(128, new char[128]));
  providers.push_back(new cu::IO::RawDataProvider(12, new char[12]));
  providers.push_back(new cu::IO::RawDataProvider(13, new char[13]));

  char x = 0;

  for (size_t i = 0; i < providers.size(); i++) {
    cu::IO::DataProvider* provider = providers[i];

    for (size_t j = 0; j < provider->Size(); j++) {
      provider->Data()[j] = x++;
    }
  }

  cu::IO::VectorDataProviderIterator providers_iterator(providers);

  x = 0;

  cu::IO::UnifyingBlockInputStream ubis(256, &providers_iterator);

  REQUIRE (ubis.HasMore());

  boost::optional< cu::Block<> > ao = ubis.Next();

  REQUIRE (ao);

  cu::Block<> a = ao.get();

  REQUIRE (a.Elements() == 256);

  for (size_t i = 0; i < a.Elements(); i++) {
    REQUIRE (*a(i) == x++);
  }

  REQUIRE (ubis.HasMore());

  boost::optional< cu::Block<> > bo = ubis.Next();

  REQUIRE (bo);

  cu::Block<> b = bo.get();

  REQUIRE (b.Elements() == 256);

  REQUIRE (a(0) != b(0));
  REQUIRE ((b(0) - a(0)) == 256);

  for (size_t i = 0; i < b.Elements(); i++) {
    REQUIRE (*b(i) == x++);
  }

  REQUIRE (ubis.HasMore());

  boost::optional< cu::Block<> > co = ubis.Next();

  REQUIRE (co);

  cu::Block<> c = co.get();

  REQUIRE (c.Elements() == 256);

  REQUIRE (a(0) != c(0));
  REQUIRE (b(0) != c(0));
  REQUIRE ((a(0) + 512) != c(0));

  for (size_t i = 0; i < 128; i++) {
    REQUIRE (*c(i) == x++);
  }

  for (size_t i = 128; i < 128 + 12; i++) {
    REQUIRE (*c(i) == x++);
  }

  for (size_t i = 128 + 12; i < 128 + 12 + 13; i++) {
    REQUIRE (*c(i) == x++);
  }

  for (size_t i = 128 + 12 + 13; i < c.Elements(); i++) {
    REQUIRE (*c(i) == 0);
  }

  REQUIRE (!ubis.HasMore());

  REQUIRE (!ubis.Next());

  for (size_t i = 0; i < providers.size(); i++) {
    delete[] providers[i]->Data();
    delete providers[i];
    providers[i] = NULL;
  }
}
