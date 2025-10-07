#include <bitset>
#include <cstdint>
#include <ctype.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <queue>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace huff {

template <typename T> void write_binary(std::ofstream &ofs, T value) {
  ofs.write(reinterpret_cast<const char *>(&value), sizeof(T));
}

template <typename T> bool read_binary(std::ifstream &ifs, T &value) {
  ifs.read(reinterpret_cast<char *>(&value), sizeof(T));
  return bool(ifs);
}

struct BitWriter {
  std::ofstream &ofs;
  unsigned char buffer = 0;
  int count = 0;

  BitWriter(std::ofstream &ofs) : ofs(ofs) {}

  void write_bit(int bit) {
    buffer = static_cast<unsigned char>((buffer << 1) | (bit & 1));
    ++count;
    if (count == 8) {
      ofs.put(static_cast<char>(buffer));
      buffer = 0;
      count = 0;
    }
  }

  // escreve "010101" como bits
  void write_bits(const std::string &bits) {
    for (char c : bits)
      write_bit(c == '1');
  }

  void flush() {
    if (count > 0) {
      buffer <<= (8 - count);
      ofs.put(static_cast<char>(buffer));
      buffer = 0;
      count = 0;
    }
  }
};

struct BitReader {
  std::ifstream &ifs;
  unsigned char buffer = 0;
  int count = 0;

  BitReader(std::ifstream &ifs) : ifs(ifs) {}

  int read_bit() {
    if (count == 0) {
      char ch;
      if (!ifs.get(ch))
        return -1; // EOF
      buffer = static_cast<unsigned char>(ch);
      count = 8;
    }
    int bit = (buffer >> (count - 1)) & 1;
    --count;
    return bit;
  }
};

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

  bool is_leaf() const { return !left && !right; }
};

struct NodeCompare {
  bool operator()(const node *a, const node *b) const {
    return a->freq > b->freq;
  }
};

void delete_tree(node *nd) {
  if (!nd)
    return;
  delete_tree(nd->left);
  delete_tree(nd->right);
  delete nd;
}

bool file_is_empty(const std::filesystem::path &p) {
  std::error_code ec;
  auto sz = std::filesystem::file_size(p, ec);
  if (!ec)
    return sz == 0;

  std::ifstream ifs(p, std::ios::binary);
  if (!ifs.is_open())
    return false;

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
  if (vec.empty())
    return nullptr;

  std::priority_queue<node *, std::vector<node *>, NodeCompare> pqueue(
      vec.begin(), vec.end());

  while (pqueue.size() > 1) {
    node *left = pqueue.top();
    pqueue.pop();
    node *right = pqueue.top();
    pqueue.pop();
    node *ptr = new node("", left->freq + right->freq, left, right);
    pqueue.push(ptr);
  }

  return pqueue.top();
}

std::string create_tb(node *nd, std::string s,
                      std::unordered_map<std::string, std::string> &map) {
  if (!nd)
    return "";
  if (nd->left)
    create_tb(nd->left, s + "0", map);
  if (nd->right)
    create_tb(nd->right, s + "1", map);
  if (!nd->symbol.empty()) {
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

// escreve árvore em pré-ordem: 0 = nó interno; 1 + 8bits = folha (símbolo)
void write_tree(node *node, BitWriter &bw) {
  if (!node)
    return;
  if (node->symbol.empty()) {
    bw.write_bit(0);
    write_tree(node->left, bw);
    write_tree(node->right, bw);
  } else {
    bw.write_bit(1);
    unsigned char c = static_cast<unsigned char>(node->symbol[0]);
    for (int i = 7; i >= 0; --i) {
      bw.write_bit((c >> i) & 1);
    }
  }
}

void count_word(std::string word,
                std::unordered_map<std::string, unsigned int> &freq) {
  std::set<std::string> keywords = {"alignas",
                                    "alignof",
                                    "and",
                                    "and_eq",
                                    "asm",
                                    "atomic_cancel",
                                    "atomic_commit",
                                    "atomic_noexcept",
                                    "auto",
                                    "bitand",
                                    "bitor",
                                    "bool",
                                    "break",
                                    "case",
                                    "catch",
                                    "char",
                                    "char8_t",
                                    "char16_t",
                                    "char32_t",
                                    "class",
                                    "compl",
                                    "concept",
                                    "const",
                                    "consteval",
                                    "constexpr",
                                    "constinit",
                                    "const_cast",
                                    "continue",
                                    "contract_assert",
                                    "co_await",
                                    "co_return",
                                    "co_yield",
                                    "decltype",
                                    "default",
                                    "delete",
                                    "do",
                                    "double",
                                    "dynamic_cast",
                                    "else",
                                    "enum",
                                    "explicit",
                                    "export",
                                    "extern",
                                    "false",
                                    "float",
                                    "for",
                                    "friend",
                                    "goto",
                                    "if",
                                    "inline",
                                    "int",
                                    "long",
                                    "mutable",
                                    "namespace",
                                    "new",
                                    "noexcept",
                                    "not",
                                    "not_eq",
                                    "nullptr",
                                    "operator",
                                    "or",
                                    "or_eq",
                                    "private",
                                    "protected",
                                    "public",
                                    "reflexpr",
                                    "register",
                                    "reinterpret_cast",
                                    "requires",
                                    "return",
                                    "short",
                                    "signed",
                                    "sizeof",
                                    "static",
                                    "static_assert",
                                    "static_cast",
                                    "struct",
                                    "switch",
                                    "synchronized",
                                    "template",
                                    "this",
                                    "thread_local",
                                    "throw",
                                    "true",
                                    "try",
                                    "typedef",
                                    "typeid",
                                    "typename",
                                    "union",
                                    "unsigned",
                                    "using",
                                    "virtual",
                                    "void",
                                    "volatile",
                                    "wchar_t",
                                    "while",
                                    "xor",
                                    "xor_eq",
                                    "std",
                                    "cin",
                                    "cout"};

  if (!word.empty()) {
    if (keywords.count(word)) {
      freq[word]++;
    } else {
      for (char character : word) {
        std::string ss(1, character);
        freq[ss]++;
      }
    }
    word.clear();
  }
}

std::unordered_map<std::string, unsigned int> count_freq(std::ifstream &file) {
  std::unordered_map<std::string, unsigned int> freq;
  std::string line, word;
  char c;
  while (file.get(c)) {
    std::string s(1, c);
    if (isalpha(c) || isdigit(c) || c == '_') {
      word.append(s);
    } else {
      count_word(word, freq);
    }
    freq[s]++;
  }

  count_word(word, freq);
  return freq;
}

void read_tree(node *&nd, BitReader &br) {
  int bit = br.read_bit();
  if (bit == -1)
    throw std::runtime_error("read_tree: EOF inesperado");

  nd = new node();
  if (bit == 1) { // folha
    unsigned char sym = 0;
    for (int i = 0; i < 8; ++i) {
      int b = br.read_bit();
      if (b == -1)
        throw std::runtime_error("read_tree: EOF inesperado ao ler símbolo");
      sym = static_cast<unsigned char>((sym << 1) | b);
    }
    nd->symbol = std::string(1, static_cast<char>(sym));
  } else { // nó interno
    read_tree(nd->left, br);
    read_tree(nd->right, br);
  }
}

void encoding(const std::string &filename) {
  std::ifstream file(filename, std::ios::binary);
  if (!file.is_open()) {
    std::cerr << "Erro ao abrir o arquivo!\n";
    return;
  }

  auto map = count_freq(file);
  if (map.empty()) {
    std::cerr << "Arquivo vazio ou sem conteúdo para comprimir.\n";
    return;
  }

  // total de símbolos (bytes) do arquivo original
  uint64_t total_symbols = 0;
  for (auto &p : map)
    total_symbols += p.second;

  auto vec = create_forest(map);
  node *root = create_tree(vec);
  if (!root) {
    std::cerr << "Erro ao criar árvore.\n";
    return;
  }

  auto freq_table = create_table(root);

  std::ofstream ofs("teste.huff", std::ios::binary);
  if (!ofs.is_open()) {
    std::cerr << "Erro ao criar arquivo de saída.\n";
    delete_tree(root);
    return;
  }

  write_binary<uint64_t>(ofs, total_symbols);

  BitWriter bw(ofs);
  write_tree(root, bw);

  std::ifstream ifs2(filename, std::ios::binary);
  if (!ifs2.is_open()) {
    std::cerr << "Erro reabrir arquivo de entrada.\n";
    ofs.close();
    delete_tree(root);
    return;
  }

  char c;
  while (ifs2.get(c)) {
    std::string s(1, c);
    auto it = freq_table.find(s);
    if (it == freq_table.end()) {
      std::cerr << "Símbolo sem código!\n";
      break;
    }
    bw.write_bits(it->second);
  }

  bw.flush();
  ofs.close();

  delete_tree(root);
}

void decoding(const std::string &filename) {
  std::ifstream ifs(filename, std::ios::binary);
  if (!ifs.is_open()) {
    std::cerr << "Erro ao abrir arquivo .huff\n";
    return;
  }

  uint64_t total_symbols = 0;
  if (!read_binary<uint64_t>(ifs, total_symbols)) {
    std::cerr << "Arquivo corrompido (sem header).\n";
    return;
  }

  BitReader br(ifs);
  node *root = nullptr;
  try {
    read_tree(root, br);
  } catch (const std::exception &e) {
    std::cerr << "Erro ao ler árvore: " << e.what() << "\n";
    return;
  }

  if (!root) {
    std::cerr << "Árvore vazia.\n";
    return;
  }

  std::ofstream ofs_out("teste.txt", std::ios::binary);
  if (!ofs_out.is_open()) {
    std::cerr << "Erro ao criar arquivo de saída.\n";
    delete_tree(root);
    return;
  }

  node *ptr = root;
  uint64_t written = 0;
  while (written < total_symbols) {
    int b = br.read_bit();
    if (b == -1) {
      std::cerr << "EOF inesperado durante decodificação\n";
      break;
    }
    ptr = (b ? ptr->right : ptr->left);
    if (!ptr) {
      std::cerr << "Estrutura da árvore corrompida durante decodificação\n";
      break;
    }
    if (ptr->is_leaf()) {
      ofs_out.put(ptr->symbol[0]);
      ptr = root;
      ++written;
    }
  }

  ofs_out.close();
  delete_tree(root);
}

void help() {}

} // namespace huff
