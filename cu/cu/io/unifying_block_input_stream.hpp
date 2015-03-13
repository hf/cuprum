// Copyright (C) 2015 Stojan Dimitrovski
//
// This file is distributed under The MIT License. Please see the enclosed file
// LICENSE.txt for the full text.

#pragma once

#include <boost/thread/lock_guard.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/lockable_adapter.hpp>
#include <boost/thread/tss.hpp>

#include <boost/optional.hpp>

#include "../block.hpp"

#include "data_provider.hpp"

namespace cu {
namespace IO {

/*
 * Unifies multiple data sources into a sequence of blocks.
 */
class UnifyingBlockInputStream :
  public boost::lockable_adapter < boost::recursive_mutex >
{
public:
  UnifyingBlockInputStream(std::size_t, DataProviderIterator*);
  ~UnifyingBlockInputStream();

  /* Size of an output block. */
  inline std::size_t BlockSize() const {
    return block_size;
  }

  /* Reset the stream, i.e. start from beginning. */
  void Reset();

  /*
   * Check if there is more input to process.
   * DO NOT USE THIS IN A WHILE LOOP!
   */
  bool HasMore();

  /*
   * Returns an empty optional if there is no more input to process, otherwise
   * returns a block of input which is guaranteed to hold BlockSize() bytes.
   *
   * You MUST NOT hold on to the data inside cu::Block. It is valid until the
   * next call of Next() in the current thread.
   */
  boost::optional< cu::Block<> > Next();

private:
  std::size_t block_size;
  DataProviderIterator* providers_iterator;

  DataProvider* current_provider;

  std::size_t position;

  boost::thread_specific_ptr<char> buffer;

  /*
   * Holds a thread-specific reference to a DataProvider which is active in
   * the current thread, so that when the provider changes in the same thread
   * (with calling Next()), it will be properly relaxed.
   */
  boost::thread_specific_ptr<DataProvider> active_provider;

  static void CleanupThreadSpecificBuffer(char*);
  static void CleanupActiveProvider(DataProvider*);

  bool Advance();
  bool Step();
};

UnifyingBlockInputStream::UnifyingBlockInputStream(std::size_t bs, DataProviderIterator* p) :
  block_size(bs),
  providers_iterator(p),
  current_provider(NULL),
  active_provider(CleanupActiveProvider),
  buffer(CleanupThreadSpecificBuffer)
{
  assert (block_size > 0);
  assert (providers_iterator != NULL);

  Reset();
}

UnifyingBlockInputStream::~UnifyingBlockInputStream() {
  // No-op.
}

void UnifyingBlockInputStream::Reset() {
  boost::lock_guard<UnifyingBlockInputStream> guard(*this);
  boost::lock_guard<DataProviderIterator> guard_providers_iterator(*providers_iterator);

  active_provider.reset(NULL);

  current_provider = NULL;

  if (!providers_iterator->Reset()) {
    return;
  }

  Step();
}

bool UnifyingBlockInputStream::HasMore() {
  boost::lock_guard<UnifyingBlockInputStream> guard(*this);

  return (current_provider != NULL && position < current_provider->Size()) || providers_iterator->HasMore();
}

boost::optional< cu::Block<> > UnifyingBlockInputStream::Next() {
  boost::lock_guard<UnifyingBlockInputStream> guard(*this);
  boost::lock_guard<DataProviderIterator> guard_providers_iterator(*providers_iterator);

  if (!HasMore()) {
    // relax the last active_provider
    active_provider.reset(NULL);

    return boost::optional< cu::Block<> >();
  }

  if (current_provider == NULL) {
    Step();
  }

  assert (current_provider != NULL);

  if (position + block_size <= current_provider->Size()) {
    char* data = current_provider->Data() + position;

    position += block_size;

    if (position == current_provider->Size()) {
      Advance();
    }

    return boost::optional< cu::Block<> >(cu::Block<>(data, block_size));
  }

  if (buffer.get() == NULL) {
    buffer.reset(new char[block_size]);
  }

  std::size_t i = 0;

  char* const buffer = this->buffer.get();

  while (i < block_size) {
    std::size_t upto = block_size - i;

    if (upto + position > current_provider->Size()) {
      upto = current_provider->Size() - position;
    }

    assert (upto + position <= current_provider->Size());
    assert (i + upto <= block_size);

    char* data = current_provider->Data() + position;

    for (std::size_t j = 0; j < upto; j++) {
      buffer[i] = data[j];
      i += 1;
    }

    position += upto;

    if (position == current_provider->Size()) {
      if (!Advance()) {
        // padding
        for (; i < block_size; i++) {
          buffer[i] = 0;
        }
      }
    }
  }

  return boost::optional< cu::Block<> > (cu::Block<>(buffer, block_size));
}

void UnifyingBlockInputStream::CleanupThreadSpecificBuffer(char* buffer) {
  if (buffer == NULL) {
    return;
  }

  delete[] buffer;
}

void UnifyingBlockInputStream::CleanupActiveProvider(DataProvider* active_provider) {
  if (active_provider == NULL) { return; }

  // DO NOT delete active_provider HERE!
  active_provider->Relax();
}

bool UnifyingBlockInputStream::Advance() {
  boost::lock_guard<UnifyingBlockInputStream> guard(*this);
  boost::lock_guard<DataProviderIterator> guard_providers_iterator(*providers_iterator);

  assert (current_provider != NULL && position == current_provider->Size());

  if (!providers_iterator->HasMore()) {
    current_provider = NULL;
    active_provider.reset(current_provider);

    return false;
  }

  return Step();
}

bool UnifyingBlockInputStream::Step() {
  boost::lock_guard<UnifyingBlockInputStream> guard(*this);

  position = 0;
  current_provider = providers_iterator->Next();

  active_provider.reset(current_provider);

  if (current_provider == NULL) {
    return false;
  }

  assert (current_provider != NULL);

  current_provider->Prepare();

  return true;
}

} // IO
} // cu
