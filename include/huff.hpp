#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <queue>
#include <string>
#include <system_error>
#include <unordered_map>
#include <vector>
class huff {
public:
  struct node {
    std::string symbol;
    unsigned int freq;
    node *left, *right;

    node(const std::string &symbol, unsigned int freq)
        : symbol(symbol), freq(freq) {}

    void print_io() {
      if (left)
        left->print_io();
      std::cout << "{" << (symbol.empty() ? "father" : symbol) << ": " << freq
                << "}\n";
      if (right)
        right->print_io();
    }
  };

  std::unordered_map<std::string, unsigned int>
  count_freq(const std::string &) const;

  void encoding(const std::string &) const;

  void decoding(const std::string &) const;

private:
  node *create_tree(const std::vector<node *> &) const;

  std::vector<huff::node *>
  create_forest(const std::unordered_map<std::string, unsigned int> &) const;

  std::unordered_map<std::string, std::string> create_table(node *) const;
};
