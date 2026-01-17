#ifndef HUFF_HPP
#define HUFF_HPP

#include <string>
#include <unordered_map>

#include "huffman_tree.hpp"
#include "trie.hpp"

namespace huff {

void encoding(const std::string &, const std::string & = "huff.config");

void decoding(const std::string &);

void help();

}; // namespace huff
#endif // !HUFF_HPP
