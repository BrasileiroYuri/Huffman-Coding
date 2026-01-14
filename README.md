# Huffman Coding Project

## 📌 Introduction

Huffman Coding is a lossless data compression algorithm widely used in file compression formats such as ZIP, GZIP, and PNG.
It works by assigning shorter binary codes to more frequent characters and longer codes to less frequent ones, minimizing the total number of bits used.

## ⚙️ How it Works (Overview)

1. Count the frequency of each symbol in the input.
2. Build a **priority queue (min-heap)** based on frequencies.
3. Construct a **binary tree** (Huffman Tree), merging nodes from lowest frequencies to highest.
4. Assign binary codes:
   - Left branch → `0`
   - Right branch → `1`
5. Encode the input using the generated codes.
6. Decode by traversing the Huffman Tree.

## ✅ Properties

- **Lossless:** The original data can always be perfectly reconstructed.
- **Prefix-free codes:** No code is a prefix of another, ensuring unambiguous decoding.
- **Optimality:** Produces the best variable-length code given known symbol frequencies.

## 📂 Project Structure

- `src/` → Source code (Huffman implementation)
- `docs/` → Documentation

## 🚀 Future Additions

- [ ] Implementation details
- [ ] Examples of encoding/decoding
- [ ] Performance analysis

---

## 🚀 Compilation

### 🔹 Using CMake

```bash
cmake -S . -B build
cmake --build build
./build/huff { -c | -d } <file>
```

### 🔹 Using g++

```bash
g++ src/*.cpp -o huff
./huff { -c | -d } <file>
```

✍️ *This README is a draft. I’ll add more details about the implementation, usage, and results as the project develops.*
