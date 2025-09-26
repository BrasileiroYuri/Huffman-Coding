#include "../include/huff.hpp"
#include <string>

struct Compare {
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

std::unordered_map<std::string, unsigned int>
huff::count_freq(const std::string &filename) const {
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

std::vector<huff::node *> huff::create_forest(
    const std::unordered_map<std::string, unsigned int> &map) const {

  std::vector<node *> vec;

  for (const auto &p : map) {
    vec.push_back(new node(p.first, p.second));
  }

  return vec;
}

huff::node *huff::create_tree(const std::vector<huff::node *> &vec) const {

  std::priority_queue<node *, std::vector<huff::node *>, Compare> pqueue(
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

std::unordered_map<std::string, std::string>
huff::create_table(node *node) const {

  std::unordered_map<std::string, std::string> map;

  if (node)
    create_tb(node, "", map);

  return map;
}

void encode_table(huff::node *root, const std::string &filename) {}

void huff::encoding(const std::string &filename) const {
  auto map = count_freq(filename);

  // Arquivo(map) vazio!
  if (map.empty()) {
    return;
  }

  std::cout << "Frequencia:\n";
  for (auto p : map) {
    std::cout << p.first << ": " << p.second << "\n";
  }
  auto vecn = create_forest(map);
  auto node = create_tree(vecn);

  node->print_io();

  auto freq_table = create_table(node);

  std::cout << "\nBits:\n";
  for (const auto p : freq_table) {
    std::cout << p.first << ": " << p.second << "\n";
  }

  encode_table(node, filename);
  //!< Criar novo arquivo .huff com o arvore de cabeçalho e o binario.
}

void huff::decoding(const std::string &filename) const {
  //!< Decodificar e recriar o arquivo inicial.
}
