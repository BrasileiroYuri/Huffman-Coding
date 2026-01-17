#include "huff.hpp"
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

std::string binary_to_string(const std::string &binary) {
  std::string frase;
  frase.reserve(binary.size());

  for (size_t i{0}; i < binary.size(); i += 8)
    frase += static_cast<char>(std::stoi(binary.substr(i, 8), nullptr, 2));

  return frase;
}

std::pair<std::string, std::string>
get_name_extension(const std::string &filename) {

  size_t dot_position = filename.find_last_of('.');
  std::string new_filename;
  if (dot_position == std::string::npos) {
    return {filename, ""};
  }
  return {filename.substr(0, dot_position), filename.substr(dot_position + 1)};
}

void encode_extension(BitWriter &bw, const std::string &extension) {

  for (const auto &c : extension) {
    std::bitset<8> bits((unsigned long)c);
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

  std::string buffer((unsigned long)size, '\0');
  file.read(&buffer[0], size);

  return buffer;
}

void encoding(const std::string &filename, const std::string &config_file) {

  trie tr;
  tr.insert_keywords(config_file);

  std::string buffer = get_buffer(filename);

  auto map = tr.count_freq(buffer);
  if (map.empty()) {
    std::cerr << "Arquivo vazio ou sem conteúdo para comprimir.\n";
    return;
  }

  // total de símbolos (bytes) do arquivo original
  uint64_t total_symbols = 0;
  for (auto &p : map)
    total_symbols += p.second;

  huffman_tree ht(map);
  auto freq_table = ht.create_table();

  auto name_ext = get_name_extension(filename);
  auto new_filename = name_ext.first + ".huff";

  std::ofstream ofs(new_filename, std::ios::binary);

  if (!ofs.is_open()) {
    std::cerr << "Erro ao criar arquivo de saída.\n";
    return;
  }

  write_binary<uint64_t>(ofs, total_symbols);
  write_binary<uint8_t>(ofs, static_cast<uint8_t>(name_ext.second.size()));

  BitWriter bw(ofs);
  encode_extension(bw, name_ext.second);

  ht.write_tree(bw);
  tr.write(bw, freq_table, buffer);
}

void decoding(const std::string &filename) {

  auto name_extension = get_name_extension(filename);

  if (name_extension.second != "huff") {
    std::cerr << "O arquivo não é um .huff.\n";
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
  std::string bits;
  bits.reserve(static_cast<size_t>(ext_size) * 8);

  size_t ext_bits = static_cast<size_t>(ext_size) * 8;
  while (ext_written < ext_bits) {
    int b = br.read_bit();

    if (b == -1) {
      std::cerr << "EOF inesperado durante decodificação.\n";
      return;
    }

    bits += (b ? '1' : '0');
    ++ext_written;
  }

  huffman_tree ht;
  ht.read_tree(br);

  auto final_filename = name_extension.first + "." + binary_to_string(bits);

  std::ofstream ofs_out(final_filename, std::ios::binary);

  if (!ofs_out.is_open()) {
    std::cerr << "Erro ao criar arquivo de saída.\n";
    return;
  }

  ht.write_symbols(total_symbols, br, ofs_out);
  ofs_out.close();
}

void help() {
  std::string message = R"(
 Usage: ./huff { -d | -c } <file>

  Options:

   -d     Decode a .huff file.
   -c     Encode a file.
)";

  std::cout << message << "\n";
}

} // namespace huff
