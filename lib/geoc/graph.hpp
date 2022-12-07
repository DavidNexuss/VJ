#pragma once

template <typename T> struct graph {
  virtual T &getValue(int nodeIndex);
  virtual int neighbor(int nodeIndex, int neighborIndex);
  virtual int neighborCount();
};

template <typename T> struct matgraph : public graph<T> {};
template <typename T> struct sparsegraph : public graph<T> {};
