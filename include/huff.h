#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

class huff {

private:
  struct node {
    std::string symbol = 0;
    unsigned int freq = 0;
    node *left;
    node *right;

    node(const std::string &symbol, unsigned int freq)
        : symbol(symbol), freq(freq) {}

    void print() { std::cout << symbol << ": " << freq << "\n"; }
  };

  node *create_tree(const std::vector<node *> &) const;

  std::vector<node *>
  create_forest(const std::map<std::string, unsigned int> &) const;

public:
  std::map<std::string, unsigned int> count_freq(const std::string &) const;

  void encoding(const std::string &) const;

  void decoding(const std::string &) const;
};
