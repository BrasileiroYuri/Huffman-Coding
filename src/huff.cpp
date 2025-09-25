#include "../include/huff.h"

struct Compare {
  bool operator()(const huff::node *a, const huff::node *b) const {
    return a->freq > b->freq;
  }
};

std::map<std::string, unsigned int>
huff::count_freq(const std::string &filename) const {
  std::ifstream file(filename);

  if (!file.is_open()) {
    std::cerr << "Erro ao abrir o arquivo!\n";
    exit(1);
  }

  std::map<std::string, unsigned int> freq;
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

std::vector<huff::node *>
huff::create_forest(const std::map<std::string, unsigned int> &map) const {

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
std::unordered_map<std::string, std::string> huff::create_table(const node *&) {
  std::unordered_map<std::string, std::string> map;

  return map;
}

void huff::encoding(const std::string &filename) const {
  auto map = count_freq(filename);
  auto vecn = create_forest(map);
  auto node = create_tree(vecn);

  //!< Criar novo arquivo .huff com o arvore de cabeçalho e o binario.
}

void huff::decoding(const std::string &filename) const {
  //!< Decodificar e recriar o arquivo inicial.
}
