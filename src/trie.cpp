#include "trie.hpp"
#include <fstream>
#include <iostream>

void trie::insert_keywords(const std::string &filename) {

  std::ifstream file(filename, std::ios::binary);
  if (!file.is_open()) {
    std::cerr << "Erro ao abrir arquivo de configuração.\n";
    exit(1);
  }

  std::string word;
  while (std::getline(file, word)) {

    auto ptr = root;

    for (const auto &c : word) {
      if (ptr->children.find(c) == ptr->children.end())
        ptr->children[c] = new node();
      ptr = ptr->children[c];
    }
    ptr->is_end = true;
  }
}

std::unordered_map<std::string, unsigned>
trie::count_freq(const std::string &buffer) const {
  std::unordered_map<std::string, unsigned> map;

  std::size_t init = 0;
  while (init < buffer.size()) {
    auto ptr = root;
    std::size_t i = init;
    int last_end = -1;

    while (i < buffer.size() &&
           ptr->children.find(buffer[i]) != ptr->children.end()) {
      ptr = ptr->children[buffer[i]];
      if (ptr->is_end)
        last_end = (int)i;
      i++;
    }

    if (last_end != -1) {
      map[buffer.substr(init, (size_t)last_end - init + 1)]++;
      init = (size_t)last_end + 1;
    } else {
      map[std::string(1, buffer[init])]++;
      init++;
    }
  }
  return map;
}
void trie::write(BitWriter &bw,
                 const std::unordered_map<std::string, std::string> &freq_table,
                 const std::string &buffer) const {

  std::size_t init = 0;
  while (init < buffer.size()) {
    auto ptr = root;

    std::size_t last_end = (size_t)-1;
    std::size_t i = init;

    while (i < buffer.size() &&
           ptr->children.find(buffer[i]) != ptr->children.end()) {
      ptr = ptr->children[buffer[i]];
      if (ptr->is_end)
        last_end = i;
      i++;
    }

    // se palavra.
    if (last_end != (size_t)-1) {
      bw.write_bits(freq_table.at(buffer.substr(init, last_end - init + 1)));
      init = last_end + 1;
    } else {
      bw.write_bits(freq_table.at(std::string(1, buffer[init++])));
    }
  }

  bw.flush();
}
