#include "include/huff.hpp"

int main(int argc, char *argv[]) {

  if (argc == 1) {
    huff::help();
  }

  huff::encoding(argv[1]);
  return 0;
}
