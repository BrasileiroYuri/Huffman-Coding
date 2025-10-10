#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <queue>
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

struct TrieNode {

  bool is_end;
  std::unordered_map<char, TrieNode *> children;

  TrieNode(bool is_end = false) : is_end(is_end) {};
};

TrieNode *root = new TrieNode();

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

struct Node {
  std::string symbol;
  unsigned int freq;
  Node *left, *right;

  Node(const std::string &symbol = "", unsigned int freq = 0,
       Node *left = nullptr, Node *right = nullptr)
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
  bool operator()(const Node *a, const Node *b) const {
    return a->freq > b->freq;
  }
};

void delete_tree(Node *nd) {
  if (!nd)
    return;
  delete_tree(nd->left);
  delete_tree(nd->right);
  delete nd;
}

Node *create_tree(const std::unordered_map<std::string, unsigned int> &map) {
  std::vector<Node *> vec;
  for (const auto &p : map) {
    vec.push_back(new Node(p.first, p.second));
  }

  if (vec.empty())
    return nullptr;

  std::priority_queue<Node *, std::vector<Node *>, NodeCompare> pqueue(
      vec.begin(), vec.end());

  while (pqueue.size() > 1) {
    Node *left = pqueue.top();
    pqueue.pop();
    Node *right = pqueue.top();
    pqueue.pop();
    Node *ptr = new Node("", left->freq + right->freq, left, right);
    pqueue.push(ptr);
  }

  return pqueue.top();
}

//===> TODO refazer em uma função só
std::string create_tb(Node *nd, std::string s,
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
std::unordered_map<std::string, std::string> create_table(Node *node) {
  std::unordered_map<std::string, std::string> map;
  if (node)
    create_tb(node, "", map);
  return map;
}
//<===

// escreve árvore em pré-ordem: 0 = nó interno; 1 + 8bits = folha (símbolo)
void write_tree(Node *node, BitWriter &bw) {
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

std::unordered_map<std::string, unsigned>
count_freq(const std::string &buffer) {
  std::unordered_map<std::string, unsigned> map;

  std::size_t init = 0;
  while (init < buffer.size()) {
    auto ptr = root;
    std::size_t i = init;
    std::size_t last_end = -1;

    while (i < buffer.size() &&
           ptr->children.find(buffer[i]) != ptr->children.end()) {
      ptr = ptr->children[buffer[i]];
      if (ptr->is_end)
        last_end = i;
      i++;
    }

    if (last_end != -1) {
      map[buffer.substr(init, last_end - init + 1)]++;
      init = last_end + 1;
    } else {
      map[std::string(1, buffer[init])]++;
      init++;
    }
  }
  return map;
}

void read_tree(Node *&nd, BitReader &br) {
  int bit = br.read_bit();
  if (bit == -1)
    throw std::runtime_error("read_tree: EOF inesperado");

  nd = new Node();
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

void insert_keywords(const std::string &filename) {

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
        ptr->children[c] = new TrieNode();
      ptr = ptr->children[c];
    }
    ptr->is_end = true;
  }
}

void encoding(const std::string &filename, const std::string &config_file) {
  insert_keywords(config_file);

  std::ifstream file(filename, std::ios::binary);
  if (!file.is_open()) {
    std::cerr << "Erro ao abrir o arquivo '" << filename << "'.\n";
    return;
  }

  file.seekg(0, std::ios::end);
  auto size = file.tellg();
  file.seekg(0, std::ios::beg);

  std::string buffer(size, '\0');
  file.read(&buffer[0], size);

  auto map = count_freq(buffer);
  if (map.empty()) {
    std::cerr << "Arquivo vazio ou sem conteúdo para comprimir.\n";
    return;
  }

  for (auto p : map) {
    std::cout << "St: '" << (p.first[0] == '\n' ? "CR" : p.first)
              << "' - Qt: " << p.second << "\n";
  }

  // total de símbolos (bytes) do arquivo original
  uint64_t total_symbols = 0;
  for (auto &p : map)
    total_symbols += p.second;

  Node *root = create_tree(map);

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

  file.clear();
  file.seekg(0, std::ios::beg);

  // TODO Buscar correspondencia de palavras
  char c;
  while (file.get(c)) {
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
  Node *root = nullptr;
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

  Node *ptr = root;
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
