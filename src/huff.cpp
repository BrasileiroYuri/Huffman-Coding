#include "../include/huff.hpp"
#include <bitset>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <queue>
#include <string>
#include <vector>

namespace huff {

struct node {
  std::string symbol;
  unsigned int freq;
  node *left, *right;

  node(const std::string &symbol = "", unsigned int freq = 0,
       node *left = nullptr, node *right = nullptr)
      : symbol(symbol), freq(freq), left(left), right(right) {}

  void print_io() const {
    if (left)
      left->print_io();
    std::cout << "{" << (symbol.empty() ? "father" : symbol) << ": " << freq
              << "}\n";
    if (right)
      right->print_io();
  }

  bool is_leaf() const { return !left and !right; }
};

struct NodeCompare {
  bool operator()(const node *a, const node *b) const {
    return a->freq > b->freq;
  }
};

bool file_is_empty(const std::filesystem::path &p) {
  std::error_code ec;
  auto sz = std::filesystem::file_size(p, ec);
  if (!ec)
    return sz == 0;

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

node *create_tree(const std::vector<node *> &vec) {

  std::priority_queue<node *, std::vector<node *>, NodeCompare> pqueue(
      vec.begin(), vec.end());

  while (pqueue.size() > 1) {
    node *right = pqueue.top();
    pqueue.pop();
    node *left = pqueue.top();
    pqueue.pop();
    node *ptr = new node("", right->freq + left->freq, left, right);
    pqueue.push(ptr);
  }

  return pqueue.top();
}

//!< TODO Como encodar as palavras-chaves?
std::string create_tb(node *nd, std::string s,
                      std::unordered_map<std::string, std::string> &map) {
  if (nd->left)
    create_tb(nd->left, s + "0", map);

  if (nd->right)
    create_tb(nd->right, s + "1", map);

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

void write_tree(node *node, std::ofstream &file) {
  if (node) {

    if (node->symbol.empty()) {
      file << '0';
      write_tree(node->left, file);
      write_tree(node->right, file);
    } else {
      file << '1';
      char c = node->symbol[0];
      for (int i = 7; i >= 0; --i) {
        file << ((c >> i) & 1 ? '1' : '0');
      }
    }
  }
}

std::unordered_map<std::string, unsigned int> count_freq(std::ifstream &file) {

  std::unordered_map<std::string, unsigned int> freq;
  std::string linha;

  //!< TODO Aprender a lidar com palavras-chaves.

  char c;
  while (file.get(c)) {
    std::string s(1, c);
    freq[s]++;
  }

  return freq;
}

void encoding(const std::string &filename) {
  std::ifstream file(filename);

  if (!file.is_open()) {
    std::cerr << "Erro ao abrir o arquivo!\n";
    exit(1);
  }

  auto map = count_freq(file);
  // Arquivo(map) vazio!
  if (map.empty()) {
    return;
  }
  auto vec = create_forest(map);
  auto node = create_tree(vec);
  auto freq_table = create_table(node);

  std::cout << "Freq: \n";
  for (auto p : map) {
    std::cout << p.first << ": " << p.second << "\n";
  }
  node->print_io();

  std::cout << "Freq_table: \n";
  for (auto p : freq_table) {
    std::cout << p.first << ": " << p.second << "\n";
  }
  //!< TODO novo do arquivo com a extensao .huff

  std::string nname;
  std::ofstream nfile("teste.huff");
  if (!nfile.is_open()) {
    return;
  }

  // Escrevendo árvore no novo arquivo .huff
  write_tree(node, nfile);
  // Abrindo novamente o arquivo pra reiniciar o ponteiro de posição.
  std::ifstream filen(filename);
  char c;

  while (filen.get(c)) {
    std::string s(1, c);
    nfile << freq_table[std::string(1, c)];
  }

  nfile.close();

  //!< TODO Limpar memória (árvore de huffman).
}

void read_tree(node *&nd, std::ifstream &file) {

  char c;
  if (!file.get(c))
    return;
  std::cout << "char c: " << c << "\n";

  nd = new node();

  if (c == '1') {
    std::string byte;
    byte.resize(8);

    for (int i = 0; i < 8; i++) {
      if (!file.get(c))
        return;
      byte[i] = c;
    }

    std::cout << "BYTE: " << byte << "\n";
    std::bitset<8> bits(byte);
    char k = static_cast<char>(bits.to_ulong());
    nd->symbol = std::string(1, k);
    std::cout << nd->symbol << "\n";

  } else {
    read_tree(nd->left, file);
    read_tree(nd->right, file);
  }
}

void decoding(const std::string &filename) {
  std::ifstream file(filename);

  //!< TODO Verificar extensao .huff

  if (!file.is_open()) {
    std::cerr << "Erro ao abrir o arquivo!\n";
    exit(1);
  } else if (file_is_empty(filename)) {
    std::cerr << "Arquivo \"" << filename << "\" vazio!\n";
  }

  node *root;
  read_tree(root, file);
  root->print_io();
  auto p = create_table(root);
  for (auto a : p) {
    std::cout << a.first << ": " << a.second << "\n";
  }

  std::ofstream nfile("teste.txt");
  if (!nfile.is_open()) {
    std::cerr << "Erro ao abrir o arquivo!\n";
    exit(1);
  }
  char c;

  node *ptr = root;

  while (file.get(c)) {
    if (c == '1') {
      // std::cout << "right\n";
      ptr = ptr->right;
    } else {
      // std::cout << "left\n";
      ptr = ptr->left;
    }
    if (ptr->is_leaf()) {
      nfile << ptr->symbol;
      ptr = root;
    }
  }

  nfile.close();
}

void help() {}
} // namespace huff
