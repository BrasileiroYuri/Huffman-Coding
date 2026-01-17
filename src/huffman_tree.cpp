#include "huffman_tree.hpp"

#include <bitset>
#include <cstdint>
#include <queue>
#include <vector>

std::string huffman_tree::create_tb(
    node *nd, std::string s,
    std::unordered_map<std::string, std::string> &map) const {
  if (!nd)
    return "";
  
  if (nd->left)
    create_tb(nd->left, s + "0", map);

  if (nd->right)
    create_tb(nd->right, s + "1", map);

  if (!nd->symbol.empty()) 
    map[nd->symbol] = s;
  
  return "";
}

std::unordered_map<std::string, std::string>
huffman_tree::create_table() const {
  std::unordered_map<std::string, std::string> map;
  if (root)
    create_tb(root, "", map);
  return map;
}
void huffman_tree::write_tree(BitWriter &bw) const { write_tree(bw, root); }

void huffman_tree::write_tree(BitWriter &bw, node *node) const {
  if (!node)
    return;
  if (node->symbol.empty()) {
    bw.write_bit(0);
    write_tree(bw, node->left);
    write_tree(bw, node->right);
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

// TODO
void huffman_tree::print() const {}

huffman_tree::huffman_tree(
    const std::unordered_map<std::string, unsigned int> &map) {
  std::vector<node *> vec;
  for (const auto &p : map) {
    vec.push_back(new node(p.first, p.second));
  }

  std::priority_queue<node *, std::vector<node *>, huffman_tree::NodeCompare>
      pqueue(vec.begin(), vec.end());

  while (pqueue.size() > 1) {
    node *left = pqueue.top();
    pqueue.pop();
    node *right = pqueue.top();
    pqueue.pop();
    node *ptr = new node("", left->freq + right->freq, left, right);
    pqueue.push(ptr);
  }

  root = pqueue.empty() ? nullptr : pqueue.top();
}
huffman_tree::~huffman_tree() {
  if (!root)
    return;

  std::queue<node *> q;
  q.push(root);

  while (!q.empty()) {
    if (q.front()->left)
      q.push(q.front()->left);

    if (q.front()->right)
      q.push(q.front()->right);

    delete q.front();
    q.pop();
  }
}

void huffman_tree::read_tree(node *&nd, BitReader &br) {
  int bit = br.read_bit();
  if (bit == -1)
    throw std::runtime_error("read_tree: EOF inesperado");

  nd = new node();
  if (bit == 1) { // folha

    uint8_t size = 0;
    for (std::size_t i = 0; i < 8; i++) {
      int b = br.read_bit();
      if (b == -1)
        throw std::runtime_error("read_tree: EOF inesperado ao ler símbolo.");
      size = (uint8_t)((size << 1) | b);
    }

    std::string word;
    uint8_t k = static_cast<uint8_t>(size);
    word.resize(k);
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

void huffman_tree::read_tree(BitReader &br) {
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
}

void huffman_tree::write_symbols(uint64_t total_symbols, BitReader &br,
                        std::ofstream &ofs_out) const {

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
      for (size_t i{0}; i < ptr->symbol.size(); ++i) {
        ofs_out.put(ptr->symbol[i]);
      }
      ptr = root;
      ++written;
    }
  }
}
