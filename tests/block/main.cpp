#define CATCH_CONFIG_MAIN

#include <catch.hpp>

#include <cu/block.hpp>

#include <cstdlib>

TEST_CASE ("Block should initialize properly.", "[cu::Block]") {
  char data[1024];

  cu::Block<> a(data, 1024);

  REQUIRE (a.First() == data);
  REQUIRE (a.Last() == data + 1023);
  REQUIRE (a.Elements() == 1024);
  REQUIRE (a.Size() == 1024);
  REQUIRE (a.Data() == data);
  REQUIRE (a(1) == data + 1);
  REQUIRE (a.Width() == 1);

  cu::Block<> b(data, 2, 1024 / 2);

  REQUIRE (b.First() == data);
  REQUIRE (b.Last() == data + 1022);
  REQUIRE (b.Elements() == 1024 / 2);
  REQUIRE (b.Size() == 1024);
  REQUIRE (b.Data() == data);
  REQUIRE (b(1) == data + 2);
  REQUIRE (b.Width() == 2);
}

TEST_CASE ("Block should return proper subrange.", "[cu::Block]") {
  char data[1024];

  cu::Block<> a(data, 1024);

  cu::Block<> asub = a.Range(0, 1024);

  REQUIRE (asub.Elements() == 1024);
  REQUIRE (asub.Data() == data);
  REQUIRE (asub.Size() == 1024);
  REQUIRE (asub.First() == data);
  REQUIRE (asub.Last() == data + 1023);
  REQUIRE (asub(1) == data + 1);

  cu::Block<> bsub = a.Range(1, 1023);

  REQUIRE (bsub.Elements() == 1022);
  REQUIRE (bsub.Data() == data + 1);
  REQUIRE (bsub.Size() == 1022);
  REQUIRE (bsub.First() == data + 1);
  REQUIRE (bsub.Last() == (data + 1) + 1021);
  REQUIRE (bsub(1) == data + 2);

  cu::Block<> b(data, 2, 1024 / 2);

  cu::Block<> csub = b.Range(0, 1024 / 2);

  REQUIRE (csub.Elements() == 1024 / 2);
  REQUIRE (csub.Data() == data);
  REQUIRE (csub.Size() == 1024);
  REQUIRE (csub.First() == data);
  REQUIRE (csub.Last() == data + 1022);
  REQUIRE (csub(1) == data + 2);

  cu::Block<> dsub = b.Range(2, 511);

  REQUIRE (dsub.Elements() == 509);
  REQUIRE (dsub.Size() == 509 * 2);
  REQUIRE (dsub.First() == data + 4);
  REQUIRE (dsub.Last() == (data + 4) + 508 * 2);
  REQUIRE (dsub(1) == (data + 4) + 2);
}

TEST_CASE ("Block should extract proper sub-block.", "[cu::Block]") {
  char data[6 * 100];

  cu::Block<> a(data, 100, 6);

  cu::Block<> asub = a.Sub(0, 1);

  REQUIRE (asub.Elements() == 100);
  REQUIRE (asub.Size() == 100);
  REQUIRE (asub.First() == data);
  REQUIRE (asub.Last() == data + 99);

  cu::Block<> bsub = a.Sub(1, 2);

  REQUIRE (bsub.Elements() == 50);
  REQUIRE (bsub.Size() == 100);
  REQUIRE (bsub.First() == data + 100);
  REQUIRE (bsub.Last() == (data + 100) + 98);
}

TEST_CASE ("Block should recast properly.", "[cu::Block]") {
  char data[1024];

  cu::Block<> a(data, 2, 1024 / 2);

  cu::Block<> r = a.Recast(4);

  REQUIRE (r.Elements() == 1024 / 4);
  REQUIRE (r.Width() == 4);
  REQUIRE (r.Size() == 1024);
  REQUIRE (r.First() == data);
  REQUIRE (r.Last() == data + 1020);
}

TEST_CASE ("Block<char* const> should exchange properly.", "[cu::Block<char* const>]") {
  const size_t data_s = 1026;

  char orig_data[data_s];
  char data[data_s];

  srand(data_s);

  for (size_t i = 0; i < data_s; i++) {
    data[i] = orig_data[i] = rand() % 256;
  }

  cu::Block<> a(data, data_s / 2, 2);

  a.Exchange(0, 1);

  for (size_t i = 0; i < data_s / 2; i++) {
    REQUIRE (data[i] == orig_data[i + data_s / 2]);
  }

  for (size_t i = data_s / 2; i < data_s; i++) {
    REQUIRE (data[i] == orig_data[i - data_s / 2]);
  }
}

TEST_CASE ("Block<char* const> should replace contents properly.", "[cu::Block<char* const>]") {
  const size_t data_s = 1026;

  char orig_data[data_s];
  char data[data_s];

  srand(data_s);

  for (size_t i = 0; i < data_s; i++) {
    orig_data[i] = rand() % 256;
    data[i] = rand() % 256;
  }

  cu::Block<> a(data, data_s / 2, 2);
  cu::Block<> b(orig_data, data_s);

  a.Replace(b);

  for (size_t i = 0; i < data_s; i++) {
    REQUIRE (data[i] == orig_data[i]);
  }
}
