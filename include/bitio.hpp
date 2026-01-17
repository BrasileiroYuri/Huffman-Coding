#ifndef BITIO_HPP
#define BITIO_HPP

#include <fstream>

struct BitWriter {
  std::ofstream &ofs;
  unsigned char buffer = 0;
  int count = 0;

  BitWriter(std::ofstream &ofs) : ofs(ofs) {}
  ~BitWriter() { ofs.close(); }

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
#endif // !BITIO_HPP
