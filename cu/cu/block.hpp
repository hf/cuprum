// Copyright (C) 2015 Stojan Dimitrovski
//
// This file is distributed under The MIT License. Please see the enclosed file
// LICENSE.txt for the full text.

#pragma once

#include <cstddef>
#include <cassert>

namespace cu {

#define CU_BLOCK_DEFAULT_DATA_TYPE char* const

/*
 * Block is a view over some type of data, which allows range-checked random
 * access over data.
 *
 * It works with arbitrary element widths, i.e. the size of the underlying data
 * is a multiple of its elements widths.
 */
template <typename DATA = CU_BLOCK_DEFAULT_DATA_TYPE>
class Block {
private:
  std::size_t width;
  std::size_t elements;
  DATA data;

public:
  Block(DATA d, std::size_t w, std::size_t e) :
    data(d),
    width(w),
    elements(e)
  {
    assert (elements > 0);
    assert (width > 0);
    assert (data != NULL);
  }

  Block(DATA d, std::size_t e) :
    data(d),
    width(1),
    elements(e)
  {
    assert (elements > 0);
    assert (data != NULL);
  }

  Block(const Block<DATA>& o) :
    data(o.data),
    width(o.width),
    elements(o.elements)
  {
    // No-op.
  }

  ~Block() {
    // No-op.
  }

  inline std::size_t Width() const {
    return width;
  }

  inline std::size_t Elements() const {
    return elements;
  }

  inline std::size_t Size() const {
    return elements * width;
  }

  inline DATA Data() const {
    return data;
  }

  inline Block<DATA> Range(std::size_t f, std::size_t t) const {
    assert (t >= f);
    assert ((t - f) <= Elements());

    return Block<DATA>(Get(f), width, t - f);
  }

  inline DATA Get(std::size_t i) const {
    return (*this)(i);
  }

  inline DATA operator()(std::size_t i) const {
    assert (i < Elements());

    return Data() + i * width;
  }

  inline Block<DATA> Sub(std::size_t i) const {
    return Sub(i, 1);
  }

  inline DATA SubGet(std::size_t i, std::size_t subwidth, std::size_t j) const {
    // TODO: Refactor this without creating aditional Block<> objects
    return Sub(i, subwidth).Get(j);
  }

  inline Block<DATA> Sub(std::size_t i, std::size_t subwidth) const {
    assert (subwidth > 0);
    assert (i < Elements());

    return Block<DATA>((*this)(i), subwidth, width / subwidth);
  }

  inline DATA First() const {
    return (*this)(0);
  }

  inline DATA Last() const {
    return (*this)(Elements() - 1);
  }

  inline Block<DATA> Recast(std::size_t width) const {
    assert (width > 0);

    return Block<DATA>(Data(), width, Size() / width);
  }

  inline void Exchange(std::size_t, std::size_t);
  inline void Replace(const Block<DATA>&);
};

template<>
void Block<>::Exchange(std::size_t i, std::size_t j) {
  assert (i < Elements());
  assert (j < Elements());

  if (i == j) {
    return;
  }

  CU_BLOCK_DEFAULT_DATA_TYPE idata = Get(i);
  CU_BLOCK_DEFAULT_DATA_TYPE jdata = Get(j);

  for (std::size_t k = 0; k < Width(); k++) {
    char t = idata[k];
    idata[k] = jdata[k];
    jdata[k] = t;
  }
}

template<>
void Block<>::Replace(const Block<>& other) {
  assert (other.Size() <= Size());

  for (std::size_t i = 0; i < other.Size(); i++) {
    *(Data() + i) = *(other.Data() + i);
  }
}

} // cu
