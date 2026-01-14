#ifndef TRIE_HPP
#define TRIE_HPP

#include <string>
#include <unordered_map>

class trie {
public:
  void insert_keywords(const std::string &filename);
  std::unordered_map<std::string, unsigned>
  count_freq(const std::string &buffer) const;

private:
  struct node {
    bool is_end;
    std::unordered_map<char, node *> children;

    node(bool is_end = false) : is_end(is_end) {};
  };

  node *root = new node();
};

#endif // !TRIE_HPP
