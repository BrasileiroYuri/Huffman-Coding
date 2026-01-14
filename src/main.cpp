#include "huff.hpp"

void options(int argc, char *argv[]) {
  if (argc == 1) {
    huff::help();
    return;
  }

  switch (argv[1][1]) {
  case 'c':
    huff::encoding(argv[2]);
    break;
  case 'd':
    huff::decoding(argv[2]);
    break;
  default:
    huff::help();
  }
}

int main(int argc, char *argv[]) {
  options(argc, argv);
  return 0;
}
