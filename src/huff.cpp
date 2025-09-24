#include "../include/huff.h"

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

//<TODO criar florestas (conjunto de arvores) de raizes.
std::vector<huff::node *>
huff::create_forest(const std::map<std::string, unsigned int> &map) const {

  std::vector<node *> vec;

  for (const auto &p : map) {
    vec.push_back(new node(p.first, p.second));
  }

  auto cmp = [](const node *a, const node *b) { return a->freq >= b->freq; };
  std::sort(vec.begin(), vec.end(), cmp);

  return vec;
}

//!< TODO Criar arvore de simbolos
huff::node *huff::create_tree(const std::vector<node *> &vec) const {
  return nullptr;
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
