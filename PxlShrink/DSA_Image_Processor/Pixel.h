#ifndef PIXEL_H
#define PIXEL_H

/**
 * Pixel struct — stores a single RGB color value.
 *
 * Each channel is an unsigned char (0–255).
 * This is a plain value type: no heap allocation,
 * so the compiler-generated copy/assign/destroy are fine.
 */
struct Pixel {
    unsigned char r;  // Red   channel
    unsigned char g;  // Green channel
    unsigned char b;  // Blue  channel

    // Default constructor — initialises to black (0, 0, 0)
    Pixel() : r(0), g(0), b(0) {}

    // Parameterized constructor
    Pixel(unsigned char r, unsigned char g, unsigned char b)
        : r(r), g(g), b(b) {}

    // Equality operator — needed later for Huffman frequency counting
    bool operator==(const Pixel& other) const {
        return r == other.r && g == other.g && b == other.b;
    }

    // Inequality operator
    bool operator!=(const Pixel& other) const {
        return !(*this == other);
    }
};

#endif // PIXEL_H
