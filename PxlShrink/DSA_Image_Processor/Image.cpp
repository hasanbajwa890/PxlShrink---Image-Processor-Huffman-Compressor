#include "Image.h"
#include <fstream>
#include <cstring>
#include <iostream>

// ================================================================
//  Constructors / Destructor / Copy (Rule of Three)
// ================================================================

Image::Image() : pixels(nullptr), width(0), height(0) {}

Image::Image(int w, int h) : width(w), height(h) {
    pixels = new Matrix<Pixel>(h, w);       // rows = height, cols = width
}

Image::Image(const Image& other) : width(other.width), height(other.height) {
    if (other.pixels)
        pixels = new Matrix<Pixel>(*other.pixels);   // deep copy via Matrix copy ctor
    else
        pixels = nullptr;
}

Image& Image::operator=(const Image& other) {
    if (this == &other) return *this;        // self-assignment guard

    delete pixels;                           // free old data

    width  = other.width;
    height = other.height;
    if (other.pixels)
        pixels = new Matrix<Pixel>(*other.pixels);
    else
        pixels = nullptr;

    return *this;
}

Image::~Image() {
    delete pixels;
}

// ================================================================
//  PPM P6 Loader — manual binary parsing, no external libs
// ================================================================
bool Image::load(const char* filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error: cannot open \"" << filename << "\"\n";
        return false;
    }

    // ---- Read magic number ("P6") ----
    char m1, m2;
    file >> m1 >> m2;
    if (m1 != 'P' || m2 != '6') {
        std::cerr << "Error: not a P6 PPM file\n";
        file.close();
        return false;
    }

    // ---- Skip whitespace / comments ----
    auto skipCommentsAndWhitespace = [&]() {
        char c;
        while (file.get(c)) {
            if (c == '#') {
                // skip entire comment line
                while (file.get(c) && c != '\n') {}
            } else if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
                continue;   // skip whitespace
            } else {
                file.putback(c);
                return;
            }
        }
    };

    skipCommentsAndWhitespace();

    // ---- Read dimensions ----
    int w, h, maxVal;
    file >> w >> h;
    skipCommentsAndWhitespace();
    file >> maxVal;

    // consume the single whitespace byte after maxVal (spec requirement)
    char ws;
    file.get(ws);

    if (w <= 0 || h <= 0 || maxVal <= 0) {
        std::cerr << "Error: invalid PPM header values\n";
        file.close();
        return false;
    }

    // ---- Allocate matrix ----
    delete pixels;
    width  = w;
    height = h;
    pixels = new Matrix<Pixel>(height, width);

    // ---- Read raw RGB data ----
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            unsigned char rgb[3];
            file.read(reinterpret_cast<char*>(rgb), 3);
            (*pixels)(i, j) = Pixel(rgb[0], rgb[1], rgb[2]);
        }
    }

    file.close();
    return true;
}

// ================================================================
//  PPM P6 Saver — writes binary PPM
// ================================================================
bool Image::save(const char* filename) const {
    if (!pixels) return false;

    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error: cannot create \"" << filename << "\"\n";
        return false;
    }

    // ---- Write header ----
    file << "P6\n" << width << " " << height << "\n255\n";

    // ---- Write raw pixel data ----
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            const Pixel& p = (*pixels)(i, j);
            unsigned char rgb[3] = { p.r, p.g, p.b };
            file.write(reinterpret_cast<const char*>(rgb), 3);
        }
    }

    file.close();
    return true;
}

// ================================================================
//  Pixel access helpers
// ================================================================

Pixel Image::getPixel(int x, int y) const {
    return (*pixels)(y, x);     // matrix is (row, col) = (y, x)
}

void Image::setPixel(int x, int y, const Pixel& p) {
    (*pixels)(y, x) = p;
}
