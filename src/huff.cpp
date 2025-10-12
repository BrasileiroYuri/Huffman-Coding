#include <bitset>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cwchar>
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

TrieNode *troot = new TrieNode();

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
    if (right)
      right->print_io();
    std::cout << "{" << (symbol.empty() ? "father" : symbol) << ": " << freq
              << "}\n";
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

void delete_trie(TrieNode *node) {
  if (!node)
    return;
  for (auto &p : node->children)
    delete_trie(p.second);
  delete node;
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

// escreve árvore em pré-ordem: 0 = nó interno ou 1 = folha (tamanho da string +
// string).
void write_tree(Node *node, BitWriter &bw) {
  if (!node)
    return;
  if (node->symbol.empty()) {
    bw.write_bit(0);
    write_tree(node->left, bw);
    write_tree(node->right, bw);
  } else {
    bw.write_bit(1);

    std::bitset<8> sz(node->symbol.size());
    bw.write_bits(sz.to_string());

    for (const auto &c : node->symbol) {
      for (int i = 7; i >= 0; --i) {
        bw.write_bit((c >> i) & 1);
      }
    }
  }
}

std::unordered_map<std::string, unsigned>
count_freq(const std::string &buffer) {
  std::unordered_map<std::string, unsigned> map;

  std::size_t init = 0;
  while (init < buffer.size()) {
    auto ptr = troot;
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

    uint8_t size = 0;
    for (std::size_t i = 0; i < 8; i++) {
      int b = br.read_bit();
      if (b == -1)
        throw std::runtime_error("read_tree: EOF inesperado ao ler símbolo.");
      size = ((size << 1) | b);
    }

    std::string word;
    uint8_t i = static_cast<uint8_t>(size);
    word.resize(i);
    // Construindo o caractere i
    for (std::size_t i = 0; i < size; i++) {
      // Construindo o byte i
      uint8_t sym = 0;
      for (std::size_t j = 0; j < 8; ++j) {
        auto b = br.read_bit();
        if (b == -1)
          throw std::runtime_error("read_tree: EOF inesperado ao ler símbolo.");
        sym = static_cast<uint8_t>((sym << 1) | b);
      }
      word[i] = static_cast<char>(sym);
    }
    nd->symbol = std::move(word);
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

    auto ptr = troot;

    for (const auto &c : word) {
      if (ptr->children.find(c) == ptr->children.end())
        ptr->children[c] = new TrieNode();
      ptr = ptr->children[c];
    }
    ptr->is_end = true;
  }
}
std::string binary_to_string(const std::string &binary) {
  std::string frase;
  frase.reserve(binary.size());

  for (size_t i{0}; i < binary.size(); i += 8)
    frase += static_cast<char>(std::stoi(binary.substr(i, 8), nullptr, 2));

  return frase;
}

std::pair<std::string, std::string>
get_name_extension(const std::string &filename) {
  size_t i{0};
  size_t dot_position = filename.find_last_of('.');
  std::string new_filename;
  if (dot_position == std::string::npos) {
    return {filename, ""};
  }
  return {filename.substr(0, dot_position), filename.substr(dot_position + 1)};
}

void encode_extension(BitWriter &bw, const std::string &extension) {

  for (const auto &c : extension) {
    std::bitset<8> bits(c);
    bw.write_bits(bits.to_string());
  }
}

std::string get_buffer(const std::string &filename) {

  std::ifstream file(filename, std::ios::binary);
  if (!file.is_open()) {
    std::cerr << "Erro ao abrir o arquivo '" << filename << "'.\n";
    exit(1);
  }

  file.seekg(0, std::ios::end);
  auto size = file.tellg();
  file.seekg(0, std::ios::beg);

  std::string buffer(size, '\0');
  file.read(&buffer[0], size);

  return buffer;
}

void encoding(const std::string &filename, const std::string &config_file) {
  insert_keywords(config_file);

  std::string buffer = get_buffer(filename);

  auto map = count_freq(buffer);
  if (map.empty()) {
    std::cerr << "Arquivo vazio ou sem conteúdo para comprimir.\n";
    return;
  }

  // total de símbolos (bytes) do arquivo original
  uint64_t total_symbols = 0;
  for (auto &p : map)
    total_symbols += p.second;

  Node *root = create_tree(map);

  root->print_io();

  if (!root) {
    std::cerr << "Erro ao criar árvore.\n";
    return;
  }

  auto freq_table = create_table(root);

  auto name_ext = get_name_extension(filename);
  auto new_filename = name_ext.first + ".huff";

  std::ofstream ofs(new_filename, std::ios::binary);
  if (!ofs.is_open()) {
    std::cerr << "Erro ao criar arquivo de saída.\n";
    delete_tree(root);
    return;
  }

  write_binary<uint64_t>(ofs, total_symbols);
  write_binary<uint8_t>(ofs, name_ext.second.size());

  BitWriter bw(ofs);
  encode_extension(bw, name_ext.second);
  write_tree(root, bw);

  std::size_t init = 0;
  while (init < buffer.size()) {
    auto ptr = troot;

    std::size_t last_end = -1;
    std::size_t i = init;

    while (i < buffer.size() &&
           ptr->children.find(buffer[i]) != ptr->children.end()) {
      ptr = ptr->children[buffer[i]];
      if (ptr->is_end)
        last_end = i;
      i++;
    }

    // se palavra.
    if (last_end != -1) {
      bw.write_bits(freq_table[buffer.substr(init, last_end - init + 1)]);
      init = last_end + 1;
    } else {
      bw.write_bits(freq_table[std::string(1, buffer[init++])]);
    }
  }

  bw.flush();
  ofs.close();

  delete_tree(root);
  delete_trie(troot);
}

void decoding(const std::string &filename) {

  auto name_extension = get_name_extension(filename);
  if (name_extension.second != "huff") {
    std::cerr << "O arquivo não é um .huff.\n";
    delete_trie(troot);
    return;
  }

  std::ifstream ifs(filename, std::ios::binary);
  if (!ifs.is_open()) {
    std::cerr << "Erro ao abrir arquivo .huff\n";
    return;
  }

  uint64_t total_symbols = 0;
  u_int8_t ext_size = 0;
  u_int8_t ext_written = 0;
  if (!read_binary<uint64_t>(ifs, total_symbols) ||
      !read_binary<u_int8_t>(ifs, ext_size)) {
    std::cerr << "Arquivo corrompido (sem header).\n";
    return;
  }

  BitReader br(ifs);
  std::string byte_bits;
  byte_bits.reserve(static_cast<size_t>(ext_size) * 8);

  size_t ext_bits = static_cast<size_t>(ext_size) * 8;
  while (ext_written < ext_bits) {
    int b = br.read_bit();
    if (b == -1) {
      std::cerr << "EOF inesperado durante decodificação.\n";
      return;
    }
    byte_bits += (b ? '1' : '0');
    ++ext_written;
  }

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

  auto final_filename =
      name_extension.first + "." + binary_to_string(byte_bits);

  std::ofstream ofs_out(final_filename, std::ios::binary);

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
      for (size_t i{0}; i < ptr->symbol.size(); ++i) {
        ofs_out.put(ptr->symbol[i]);
      }
      ptr = root;
      ++written;
    }
  }

  ofs_out.close();
  delete_tree(root);
  delete_trie(troot);
}

void help() {}

} // namespace huff
