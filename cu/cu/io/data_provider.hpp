// Copyright (C) 2015 Stojan Dimitrovski
//
// This file is distributed under The MIT License. Please see the enclosed file
// LICENSE.txt for the full text.

#pragma once

#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/lockable_adapter.hpp>
#include <boost/thread/lock_guard.hpp>

#include <cstddef>

namespace cu {
namespace IO {

class DataProvider {
public:
  DataProvider() {
    #ifdef CU_TEST
    prepare_relax = 0;
    #endif
  }

  virtual ~DataProvider() {
    #ifdef CU_TEST
    assert (prepare_relax == 0);
    #endif
  }

  virtual void Prepare() {
    #ifdef CU_TEST
    prepare_relax++;
    #endif
  }

  virtual void Relax() {
    #ifdef CU_TEST
    prepare_relax--;
    #endif
  }

  virtual char* Data() const = 0;
  virtual std::size_t Size() const = 0;

private:
  #ifdef CU_TEST
  int prepare_relax;
  #endif
};

class RawDataProvider :
  public DataProvider
{
private:
  std::size_t size;
  char* const data;

public:
  RawDataProvider(std::size_t s, char* const d) :
    DataProvider::DataProvider(),
    size(s),
    data(d)
  {
    // No-op.
  }

  virtual ~RawDataProvider() {
    // No-op.
  }

  virtual char* Data() const {
    return data;
  }

  virtual std::size_t Size() const {
    return size;
  }
};

class DataProviderIterator :
  public boost::lockable_adapter < boost::recursive_mutex >
{
public:
  DataProviderIterator() {
    // No-op.
  }

  virtual ~DataProviderIterator() {
    // No-op.
  }

  #define synchronized

  /* Returns true if there is more input to process, false otherwise. */
  synchronized virtual bool HasMore() = 0;

  /* Returns a valid pointer to a DataProvider, NULL if HasMore() == false. */
  synchronized virtual DataProvider* Next() = 0;

  /* Resets this iterator to the beginning. */
  synchronized virtual bool Reset() = 0;
};

class VectorDataProviderIterator :
  public cu::IO::DataProviderIterator
{
private:
  std::vector<cu::IO::DataProvider*> data;
  std::size_t index;

public:
  VectorDataProviderIterator(const std::vector<cu::IO::DataProvider*>& d) :
    index(0),
    data(d),
    cu::IO::DataProviderIterator::DataProviderIterator()
  {
    // No-op.
  }

  virtual ~VectorDataProviderIterator() {
    // No-op.
  }

  virtual bool HasMore() {
    boost::lock_guard<DataProviderIterator> guard(*this);

    return index < data.size();
  }

  virtual cu::IO::DataProvider* Next() {
    boost::lock_guard<DataProviderIterator> guard(*this);

    if (!HasMore()) {
      return NULL;
    }

    return data[index++];
  }

  virtual bool Reset() {
    boost::lock_guard<DataProviderIterator> guard(*this);

    index = 0;

    return HasMore();
  }
};

} // IO
} // cu
