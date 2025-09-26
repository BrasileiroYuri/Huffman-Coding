#include "../include/huff.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <queue>
#include <vector>

namespace huff {

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

struct NodeCompare {
  bool operator()(const huff::node *a, const huff::node *b) const {
    return a->freq > b->freq;
  }
};

// by gpt
bool file_is_empty(const std::filesystem::path &p) {
  // 1) tenta filesystem (mais rápido)
  std::error_code ec;
  auto sz = std::filesystem::file_size(p, ec);
  if (!ec)
    return sz == 0;

  // 2) fallback abrindo stream e usando seek/tell
  std::ifstream ifs(p, std::ios::binary);
  if (!ifs.is_open())
    return false; // ou lança / trata como erro

  ifs.clear();
  ifs.seekg(0, std::ios::end);
  auto pos = ifs.tellg();
  return (pos == 0);
}

std::vector<node *>
create_forest(const std::unordered_map<std::string, unsigned int> &map) {

  std::vector<node *> vec;

  for (const auto &p : map) {
    vec.push_back(new node(p.first, p.second));
  }

  return vec;
}

node *create_tree(const std::vector<huff::node *> &vec) {

  std::priority_queue<node *, std::vector<huff::node *>, NodeCompare> pqueue(
      vec.begin(), vec.end());

  while (pqueue.size() > 1) {
    node *right = pqueue.top();
    pqueue.pop();
    node *left = pqueue.top();
    pqueue.pop();
    node *ptr = new node("", right->freq + left->freq);
    ptr->right = right;
    ptr->left = left;
    pqueue.push(ptr);
  }

  return pqueue.top();
}

std::string create_tb(huff::node *nd, std::string s,
                      std::unordered_map<std::string, std::string> &map) {
  if (nd->left)
    create_tb(nd->left, "0" + s, map);

  if (nd->right)
    create_tb(nd->right, "1" + s, map);

  // Evitar o nó raiz (vazio).
  if (nd->symbol != "") {
    map[nd->symbol] = s;
  }

  return "";
}

std::unordered_map<std::string, std::string> create_table(node *node) {

  std::unordered_map<std::string, std::string> map;

  if (node)
    create_tb(node, "", map);

  return map;
}

void encode_tree(huff::node *root, const std::string &filename) {}

std::unordered_map<std::string, unsigned int>
count_freq(const std::string &filename) {
  std::ifstream file(filename);

  if (!file.is_open()) {
    std::cerr << "Erro ao abrir o arquivo!\n";
    exit(1);
  } else if (file_is_empty(filename)) {
    std::cerr << "Arquivo \"" << filename << "\" vazio!\n";
  }

  std::unordered_map<std::string, unsigned int> freq;
  std::string linha;

  //!< TODO Aprender a lidar com palavras-chaves.

  while (std::getline(file, linha)) {
    for (char c : linha) {
      std::string s(1, c);
      freq[s]++;
    }
  }

  return freq;
}
void encoding(const std::string &filename) {
  auto map = count_freq(filename);

  // Arquivo(map) vazio!
  if (map.empty()) {
    return;
  }

  auto vecn = create_forest(map);
  auto node = create_tree(vecn);

  auto freq_table = create_table(node);

  std::cout << "Frequencia:\n";
  for (auto p : map) {
    std::cout << p.first << ": " << p.second << "\n";
  }
  node->print_io();
  std::cout << "\nBits:\n";
  for (const auto p : freq_table) {
    std::cout << p.first << ": " << p.second << "\n";
  }

  encode_tree(node, filename);
  //!< Criar novo arquivo .huff com o arvore de cabeçalho e o binario.
}

void decoding(const std::string &filename) {
  //!< Decodificar e recriar o arquivo inicial.
}

void help() {}
} // namespace huff
