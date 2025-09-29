#include "include/huff.hpp"
// TODO

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
  }
}

int main(int argc, char *argv[]) {
  options(argc, argv);
  return 0;
}
