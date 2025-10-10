#include <string>
#include <unordered_map>

#ifndef HUFF_HPP
#define HUFF_HPP

namespace huff {

std::unordered_map<std::string, unsigned int> count_freq(const std::string &);

void encoding(const std::string &, const std::string & = "huff.config");

void decoding(const std::string &);

void help();

}; // namespace huff
#endif // !HUFF_HPP
