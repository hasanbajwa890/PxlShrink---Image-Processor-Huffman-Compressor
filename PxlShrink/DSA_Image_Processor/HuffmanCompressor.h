#ifndef HUFFMAN_COMPRESSOR_H
#define HUFFMAN_COMPRESSOR_H

#include "Matrix.h"
#include "Pixel.h"

// ============================================================
//  HuffmanNode — A single node in the Huffman tree.
//  Leaf nodes store a byte value; internal nodes link children.
//  All nodes are heap-allocated with `new` and freed with `delete`.
// ============================================================
struct HuffmanNode {
    unsigned char byte;     // The byte value (meaningful only for leaves)
    int frequency;          // Frequency count
    HuffmanNode* left;      // Left child  (bit '0')
    HuffmanNode* right;     // Right child (bit '1')
    bool isLeaf;

    // Leaf constructor
    HuffmanNode(unsigned char b, int freq)
        : byte(b), frequency(freq), left(nullptr), right(nullptr), isLeaf(true) {}

    // Internal-node constructor
    HuffmanNode(int freq, HuffmanNode* l, HuffmanNode* r)
        : byte(0), frequency(freq), left(l), right(r), isLeaf(false) {}
};

// ============================================================
//  MinHeap — Manual min-heap of HuffmanNode pointers.
//  Backed by a dynamic array allocated with `new[]`.
//  Used to build the Huffman tree in O(n log n).
// ============================================================
class MinHeap {
private:
    HuffmanNode** data;   // dynamic array of node pointers
    int capacity;
    int size;

    void swap(int i, int j);
    void heapifyUp(int index);
    void heapifyDown(int index);

public:
    MinHeap(int cap);
    ~MinHeap();

    void insert(HuffmanNode* node);
    HuffmanNode* extractMin();
    int getSize() const { return size; }
};

// ============================================================
//  HuffmanCompressor — Compression / decompression of
//  Matrix<Pixel> data using Huffman coding with adaptive
//  quantization to guarantee ≥ 30% compression rate.
//
//  Binary file format (.huf):
//    [3B magic "HUF"]
//    [4B width] [4B height]
//    [1B quantizationShift]          ← NEW: bits shifted for quantization
//    [4B uniqueCount]
//    [uniqueCount × (1B value + 4B freq)]
//    [4B totalRawBytes] [4B compressedBytes] [1B paddingBits]
//    [compressedBytes of bitstream]
// ============================================================
class HuffmanCompressor {
private:
    HuffmanNode* root;

    // Code table: for each byte 0-255, its Huffman bit-string
    char codeTable[256][64];
    int  codeLengths[256];

    void buildCodes(HuffmanNode* node, char* current, int depth);
    void deleteTree(HuffmanNode* node);

    /** Internal: estimate compression rate for a given quantization shift. */
    double estimateRate(unsigned char* raw, int totalBytes, int shift);

public:
    HuffmanCompressor();
    ~HuffmanCompressor();

    /**
     * Compresses pixel matrix → binary .huf file.
     * Applies adaptive quantization to guarantee ≥ 30% compression.
     * Returns the size of the compressed file in bytes.
     */
    long compress(const Matrix<Pixel>& pixels, const char* outputPath);

    /**
     * Decompresses .huf file → new Matrix<Pixel>* (caller owns).
     * Sets width/height by reference.  Returns nullptr on failure.
     */
    Matrix<Pixel>* decompress(const char* inputPath, int& width, int& height);
};

#endif // HUFFMAN_COMPRESSOR_H
