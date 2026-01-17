
#ifndef HUFFMAN_TREE_HPP
#define HUFFMAN_TREE_HPP

#include "bitio.hpp"
#include <iostream>
#include <stdint.h>
#include <string.h>
#include <unordered_map>

class huffman_tree {
public:
  huffman_tree(const std::unordered_map<std::string, unsigned int> &map);
  huffman_tree() : root(nullptr) {}

  ~huffman_tree();

  std::unordered_map<std::string, std::string> create_table() const;
  void write_tree(BitWriter &bw) const;
  void read_tree(BitReader &br);
  void write_symbols(uint64_t total_symbols, BitReader &br,
            std::ofstream &ofs_out) const;

  // private:
  struct node {
    std::string symbol;
    unsigned int freq;
    node *left, *right;

    node(const std::string &symbol = "", unsigned int freq = 0,
         node *left = nullptr, node *right = nullptr)
        : symbol(symbol), freq(freq), left(left), right(right) {}

    bool is_leaf() const { return !left && !right; }
  };

  struct NodeCompare {
    bool operator()(const node *a, const node *b) const {
      return a->freq > b->freq;
    }
  };

  void write_tree(BitWriter &bw, node *node) const;
  void read_tree(node *&nd, BitReader &br);

  std::string
  create_tb(node *nd, std::string s,
            std::unordered_map<std::string, std::string> &map) const;

  void print() const;
  node *root;
};

#endif // !HUFFMAN_TREE
