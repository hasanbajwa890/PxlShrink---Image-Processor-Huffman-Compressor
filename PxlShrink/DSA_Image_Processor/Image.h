#ifndef IMAGE_H
#define IMAGE_H

#include "Matrix.h"
#include "Pixel.h"

/**
 * Image — Wraps Matrix<Pixel> with PPM (P6) file I/O.
 *
 * Memory contract:
 *   - Internal Matrix<Pixel>* is heap-allocated with `new`.
 *   - Destructor frees with `delete`.
 *   - Copy ctor / operator= perform deep copies.
 *   - No external image libraries (pure <fstream>).
 */
class Image {
private:
    Matrix<Pixel>* pixels;   // heap-allocated matrix (new / delete)
    int width;
    int height;

public:
    Image();
    Image(int w, int h);
    Image(const Image& other);              // deep copy
    Image& operator=(const Image& other);   // deep copy
    ~Image();

    // ---- File I/O (binary PPM P6) ----
    bool load(const char* filename);
    bool save(const char* filename) const;

    // ---- Pixel access ----
    Pixel  getPixel(int x, int y) const;
    void   setPixel(int x, int y, const Pixel& p);

    // ---- Queries ----
    int  getWidth()    const { return width;  }
    int  getHeight()   const { return height; }
    long getDataSize() const { return (long)width * height * 3; }
    bool isLoaded()    const { return pixels != nullptr; }

    // ---- Matrix access (for algorithms & compressor) ----
    const Matrix<Pixel>& getMatrix() const { return *pixels; }
    Matrix<Pixel>&       getMatrix()       { return *pixels; }
};

#endif // IMAGE_H
