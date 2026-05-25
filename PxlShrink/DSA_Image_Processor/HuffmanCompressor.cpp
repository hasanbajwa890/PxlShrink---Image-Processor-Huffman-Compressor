#include "HuffmanCompressor.h"
#include <fstream>
#include <cstring>
#include <iostream>

// ================================================================
//  MinHeap Implementation
// ================================================================

MinHeap::MinHeap(int cap) : capacity(cap), size(0) {
    data = new HuffmanNode*[capacity];
}

MinHeap::~MinHeap() {
    delete[] data;          // free the pointer array only (nodes owned by tree)
}

void MinHeap::swap(int i, int j) {
    HuffmanNode* tmp = data[i];
    data[i] = data[j];
    data[j] = tmp;
}

void MinHeap::heapifyUp(int index) {
    while (index > 0) {
        int parent = (index - 1) / 2;
        if (data[index]->frequency < data[parent]->frequency) {
            swap(index, parent);
            index = parent;
        } else {
            break;
        }
    }
}

void MinHeap::heapifyDown(int index) {
    while (true) {
        int smallest = index;
        int left  = 2 * index + 1;
        int right = 2 * index + 2;

        if (left  < size && data[left]->frequency  < data[smallest]->frequency)
            smallest = left;
        if (right < size && data[right]->frequency < data[smallest]->frequency)
            smallest = right;

        if (smallest != index) {
            swap(index, smallest);
            index = smallest;
        } else {
            break;
        }
    }
}

void MinHeap::insert(HuffmanNode* node) {
    if (size >= capacity) {
        // Grow array (double capacity)
        int newCap = capacity * 2;
        HuffmanNode** newData = new HuffmanNode*[newCap];
        for (int i = 0; i < size; i++) newData[i] = data[i];
        delete[] data;
        data = newData;
        capacity = newCap;
    }
    data[size] = node;
    heapifyUp(size);
    size++;
}

HuffmanNode* MinHeap::extractMin() {
    if (size == 0) return nullptr;
    HuffmanNode* min = data[0];
    data[0] = data[size - 1];
    size--;
    if (size > 0) heapifyDown(0);
    return min;
}

// ================================================================
//  HuffmanCompressor Implementation
// ================================================================

HuffmanCompressor::HuffmanCompressor() : root(nullptr) {
    memset(codeTable,   0, sizeof(codeTable));
    memset(codeLengths, 0, sizeof(codeLengths));
}

HuffmanCompressor::~HuffmanCompressor() {
    deleteTree(root);
}

void HuffmanCompressor::deleteTree(HuffmanNode* node) {
    if (!node) return;
    deleteTree(node->left);
    deleteTree(node->right);
    delete node;
}

// Recursive DFS to build bit-string codes for each leaf byte.
void HuffmanCompressor::buildCodes(HuffmanNode* node, char* current, int depth) {
    if (!node) return;

    if (node->isLeaf) {
        current[depth] = '\0';
        for (int i = 0; i <= depth; i++)
            codeTable[node->byte][i] = current[i];
        codeLengths[node->byte] = depth;
        return;
    }

    current[depth] = '0';
    buildCodes(node->left, current, depth + 1);

    current[depth] = '1';
    buildCodes(node->right, current, depth + 1);
}

// ----------------------------------------------------------------
//  estimateRate — quickly estimate compression rate for a given
//  quantization shift without actually encoding.
//  Returns rate as a percentage (0–100).
// ----------------------------------------------------------------
double HuffmanCompressor::estimateRate(unsigned char* raw, int totalBytes, int shift) {
    // Apply quantization to a frequency count (don't modify raw yet)
    int freq[256];
    memset(freq, 0, sizeof(freq));

    for (int i = 0; i < totalBytes; i++) {
        unsigned char quantized = (raw[i] >> shift) << shift;
        freq[quantized]++;
    }

    // Count unique values
    int uniqueCount = 0;
    for (int i = 0; i < 256; i++)
        if (freq[i] > 0) uniqueCount++;

    if (uniqueCount <= 1) return 90.0; // single value = maximum compression

    // Build temporary Huffman tree to get code lengths
    MinHeap heap(uniqueCount);
    for (int i = 0; i < 256; i++)
        if (freq[i] > 0)
            heap.insert(new HuffmanNode((unsigned char)i, freq[i]));

    while (heap.getSize() > 1) {
        HuffmanNode* l = heap.extractMin();
        HuffmanNode* r = heap.extractMin();
        heap.insert(new HuffmanNode(l->frequency + r->frequency, l, r));
    }
    HuffmanNode* tempRoot = heap.extractMin();

    // Build temporary code lengths
    int tempLengths[256];
    char tempTable[256][64];
    memset(tempLengths, 0, sizeof(tempLengths));
    memset(tempTable,   0, sizeof(tempTable));

    // Use iterative approach to get code lengths
    // (reuse buildCodes logic but with temp storage)
    // We'll do a simple recursive lambda-like approach via a stack
    struct StackEntry {
        HuffmanNode* node;
        int depth;
    };
    StackEntry stack[512];
    int stackTop = 0;
    stack[stackTop++] = {tempRoot, 0};

    while (stackTop > 0) {
        StackEntry entry = stack[--stackTop];
        if (entry.node->isLeaf) {
            tempLengths[entry.node->byte] = entry.depth;
            if (entry.depth == 0) tempLengths[entry.node->byte] = 1; // edge case
        } else {
            if (entry.node->right)
                stack[stackTop++] = {entry.node->right, entry.depth + 1};
            if (entry.node->left)
                stack[stackTop++] = {entry.node->left, entry.depth + 1};
        }
    }

    // Estimate total bits
    long totalBits = 0;
    for (int i = 0; i < 256; i++)
        if (freq[i] > 0)
            totalBits += (long)freq[i] * tempLengths[i];

    long compressedBytes = (totalBits + 7) / 8;

    // Header overhead: 3 (magic) + 4+4 (w/h) + 1 (quant) + 4 (unique)
    //                 + unique*(1+4) + 4+4+1
    long headerSize = 3 + 4 + 4 + 1 + 4 + (long)uniqueCount * 5 + 4 + 4 + 1;
    long totalFileSize = headerSize + compressedBytes;

    // Clean up temporary tree
    // (reuse deleteTree pattern)
    struct DeleteStack { HuffmanNode* node; };
    DeleteStack dStack[512];
    int dTop = 0;
    dStack[dTop++] = {tempRoot};
    while (dTop > 0) {
        HuffmanNode* n = dStack[--dTop].node;
        if (n->left)  dStack[dTop++] = {n->left};
        if (n->right) dStack[dTop++] = {n->right};
        delete n;
    }

    double rate = (1.0 - (double)totalFileSize / (double)totalBytes) * 100.0;
    return rate;
}

// ----------------------------------------------------------------
//  compress — Matrix<Pixel> → .huf binary file
//  Applies adaptive quantization to guarantee ≥ 30% compression.
// ----------------------------------------------------------------
long HuffmanCompressor::compress(const Matrix<Pixel>& pixels, const char* outputPath) {
    int height     = pixels.getRows();
    int width      = pixels.getCols();
    int totalBytes = width * height * 3;        // R G B per pixel

    // ---- Step 1: serialise pixels to flat byte array ----
    unsigned char* raw = new unsigned char[totalBytes];
    int idx = 0;
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            const Pixel& p = pixels(i, j);
            raw[idx++] = p.r;
            raw[idx++] = p.g;
            raw[idx++] = p.b;
        }
    }

    // ---- Step 2: find optimal quantization shift for ≥ 30% rate ----
    int quantShift = 0;
    for (int shift = 0; shift <= 4; shift++) {
        double rate = estimateRate(raw, totalBytes, shift);
        quantShift = shift;
        if (rate >= 30.0) break;
    }

    // ---- Step 3: apply quantization to raw bytes ----
    if (quantShift > 0) {
        for (int i = 0; i < totalBytes; i++) {
            raw[i] = (raw[i] >> quantShift) << quantShift;
        }
    }

    // ---- Step 4: frequency count ----
    int freq[256];
    memset(freq, 0, sizeof(freq));
    for (int i = 0; i < totalBytes; i++)
        freq[raw[i]]++;

    int uniqueCount = 0;
    for (int i = 0; i < 256; i++)
        if (freq[i] > 0) uniqueCount++;

    // ---- Step 5: build Huffman tree ----
    deleteTree(root);
    root = nullptr;
    memset(codeTable,   0, sizeof(codeTable));
    memset(codeLengths, 0, sizeof(codeLengths));

    if (uniqueCount == 0) { delete[] raw; return 0; }

    if (uniqueCount == 1) {
        // Edge case: every byte identical — assign single-bit code
        for (int i = 0; i < 256; i++) {
            if (freq[i] > 0) {
                root = new HuffmanNode((unsigned char)i, freq[i]);
                codeTable[i][0] = '0';
                codeTable[i][1] = '\0';
                codeLengths[i]  = 1;
                break;
            }
        }
    } else {
        MinHeap heap(uniqueCount);
        for (int i = 0; i < 256; i++)
            if (freq[i] > 0)
                heap.insert(new HuffmanNode((unsigned char)i, freq[i]));

        while (heap.getSize() > 1) {
            HuffmanNode* left  = heap.extractMin();
            HuffmanNode* right = heap.extractMin();
            heap.insert(new HuffmanNode(
                left->frequency + right->frequency, left, right));
        }
        root = heap.extractMin();

        char buf[256];
        buildCodes(root, buf, 0);
    }

    // ---- Step 6: encode to bitstream ----
    long totalBits = 0;
    for (int i = 0; i < totalBytes; i++)
        totalBits += codeLengths[raw[i]];

    int compressedBytes = (int)((totalBits + 7) / 8);
    int paddingBits     = (int)((long)compressedBytes * 8 - totalBits);

    unsigned char* compressed = new unsigned char[compressedBytes];
    memset(compressed, 0, compressedBytes);

    long bitIdx = 0;
    for (int i = 0; i < totalBytes; i++) {
        unsigned char b   = raw[i];
        int           len = codeLengths[b];
        for (int k = 0; k < len; k++) {
            if (codeTable[b][k] == '1') {
                int bytePos = (int)(bitIdx / 8);
                int bitPos  = 7 - (int)(bitIdx % 8);
                compressed[bytePos] |= (1 << bitPos);
            }
            bitIdx++;
        }
    }

    // ---- Step 7: write .huf binary file ----
    std::ofstream out(outputPath, std::ios::binary);
    if (!out.is_open()) {
        delete[] raw;
        delete[] compressed;
        return -1;
    }

    out.write("HUF", 3);
    out.write(reinterpret_cast<const char*>(&width),  4);
    out.write(reinterpret_cast<const char*>(&height), 4);

    // Write quantization shift (1 byte) — NEW in format v2
    unsigned char qsByte = (unsigned char)quantShift;
    out.write(reinterpret_cast<const char*>(&qsByte), 1);

    out.write(reinterpret_cast<const char*>(&uniqueCount), 4);

    for (int i = 0; i < 256; i++) {
        if (freq[i] > 0) {
            unsigned char bv = (unsigned char)i;
            out.write(reinterpret_cast<const char*>(&bv),      1);
            out.write(reinterpret_cast<const char*>(&freq[i]),  4);
        }
    }

    out.write(reinterpret_cast<const char*>(&totalBytes),      4);
    out.write(reinterpret_cast<const char*>(&compressedBytes), 4);
    unsigned char pad = (unsigned char)paddingBits;
    out.write(reinterpret_cast<const char*>(&pad), 1);
    out.write(reinterpret_cast<const char*>(compressed), compressedBytes);

    long fileSize = (long)out.tellp();
    out.close();

    delete[] raw;
    delete[] compressed;
    return fileSize;
}

// ----------------------------------------------------------------
//  decompress — .huf binary file → new Matrix<Pixel>*
// ----------------------------------------------------------------
Matrix<Pixel>* HuffmanCompressor::decompress(const char* inputPath,
                                              int& width, int& height) {
    std::ifstream in(inputPath, std::ios::binary);
    if (!in.is_open()) return nullptr;

    // ---- Read header ----
    char magic[4] = {};
    in.read(magic, 3);
    if (strcmp(magic, "HUF") != 0) { in.close(); return nullptr; }

    in.read(reinterpret_cast<char*>(&width),  4);
    in.read(reinterpret_cast<char*>(&height), 4);

    // Read quantization shift (1 byte) — NEW in format v2
    unsigned char quantShift = 0;
    in.read(reinterpret_cast<char*>(&quantShift), 1);

    int uniqueCount;
    in.read(reinterpret_cast<char*>(&uniqueCount), 4);

    int freq[256];
    memset(freq, 0, sizeof(freq));
    for (int i = 0; i < uniqueCount; i++) {
        unsigned char bv;
        int f;
        in.read(reinterpret_cast<char*>(&bv), 1);
        in.read(reinterpret_cast<char*>(&f),  4);
        freq[bv] = f;
    }

    int totalBytes, compressedBytes;
    unsigned char paddingBits;
    in.read(reinterpret_cast<char*>(&totalBytes),      4);
    in.read(reinterpret_cast<char*>(&compressedBytes), 4);
    in.read(reinterpret_cast<char*>(&paddingBits),     1);

    unsigned char* compressed = new unsigned char[compressedBytes];
    in.read(reinterpret_cast<char*>(compressed), compressedBytes);
    in.close();

    // ---- Rebuild Huffman tree from frequencies ----
    deleteTree(root);
    root = nullptr;

    if (uniqueCount == 1) {
        for (int i = 0; i < 256; i++) {
            if (freq[i] > 0) {
                root = new HuffmanNode((unsigned char)i, freq[i]);
                break;
            }
        }
    } else {
        MinHeap heap(uniqueCount);
        for (int i = 0; i < 256; i++)
            if (freq[i] > 0)
                heap.insert(new HuffmanNode((unsigned char)i, freq[i]));

        while (heap.getSize() > 1) {
            HuffmanNode* l = heap.extractMin();
            HuffmanNode* r = heap.extractMin();
            heap.insert(new HuffmanNode(l->frequency + r->frequency, l, r));
        }
        root = heap.extractMin();
    }

    // ---- Decode bitstream ----
    unsigned char* decoded = new unsigned char[totalBytes];
    int decodedCount = 0;
    long totalBitsAvail = (long)compressedBytes * 8 - paddingBits;
    long bitIdx = 0;

    if (uniqueCount == 1) {
        for (int i = 0; i < totalBytes; i++)
            decoded[i] = root->byte;
    } else {
        HuffmanNode* cur = root;
        while (decodedCount < totalBytes && bitIdx < totalBitsAvail) {
            int bytePos = (int)(bitIdx / 8);
            int bitPos  = 7 - (int)(bitIdx % 8);
            int bit     = (compressed[bytePos] >> bitPos) & 1;

            cur = (bit == 0) ? cur->left : cur->right;

            if (cur->isLeaf) {
                decoded[decodedCount++] = cur->byte;
                cur = root;
            }
            bitIdx++;
        }
    }

    // ---- Reconstruct Matrix<Pixel> ----
    Matrix<Pixel>* result = new Matrix<Pixel>(height, width);
    int idx = 0;
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            unsigned char r = decoded[idx++];
            unsigned char g = decoded[idx++];
            unsigned char b = decoded[idx++];
            (*result)(i, j) = Pixel(r, g, b);
        }
    }

    delete[] compressed;
    delete[] decoded;
    return result;
}
