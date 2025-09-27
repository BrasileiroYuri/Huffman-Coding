#include "include/huff.hpp"

void options(int argc, char *argv[]) {
  if (argc == 1) {
    huff::help();
    return;
  }

  for (auto i{1}; i < argc; i++) {
    if (1) {
    }
  }
}

int main(int argc, char *argv[]) {
  options(argc, argv);
  return 0;
}
