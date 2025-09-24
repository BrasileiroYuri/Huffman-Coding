#include "include/huff.h"
#include <iostream>
#include <string>

int main(int argc, char *argv[]) {

  huff coding;
  auto map = coding.count_freq(argv[1]);
  for (auto p : map) {
    std::cout << p.first << ": " << p.second << "\n";
  }

  return 0;
}
