#include <string>
#include <unordered_map>

#ifndef HUFF_HPP
#define HUFF_HPP

namespace huff {

std::unordered_map<std::string, unsigned int> count_freq(std::ifstream &);

void encoding(const std::string &);

void decoding(const std::string &);

void help();

}; // namespace huff
#endif // !HUFF_HPP
