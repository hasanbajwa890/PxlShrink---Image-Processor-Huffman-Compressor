#include "ImageProcessor.h"
#include <cmath>

// ================================================================
//  Grayscale — ITU-R BT.601 luminance formula
//  gray = 0.299·R + 0.587·G + 0.114·B
// ================================================================
Image ImageProcessor::toGrayscale(const Image& src) {
    int w = src.getWidth();
    int h = src.getHeight();
    Image result(w, h);

    const Matrix<Pixel>& srcMat = src.getMatrix();
    Matrix<Pixel>&       dstMat = result.getMatrix();

    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            const Pixel& p = srcMat(i, j);
            unsigned char gray = (unsigned char)(
                0.299 * p.r + 0.587 * p.g + 0.114 * p.b);
            dstMat(i, j) = Pixel(gray, gray, gray);
        }
    }
    return result;
}

// ================================================================
//  Sobel Edge Detection
//  Uses two 3×3 Matrix<int> kernels (Gx, Gy).
//  Works on grayscale luminance; outputs gradient magnitude.
// ================================================================
Image ImageProcessor::sobelEdgeDetect(const Image& src) {
    int w = src.getWidth();
    int h = src.getHeight();

    // ---- Build Sobel kernels using Matrix<int> ----
    Matrix<int> Gx(3, 3);
    Gx(0,0) = -1;  Gx(0,1) = 0;  Gx(0,2) = 1;
    Gx(1,0) = -2;  Gx(1,1) = 0;  Gx(1,2) = 2;
    Gx(2,0) = -1;  Gx(2,1) = 0;  Gx(2,2) = 1;

    Matrix<int> Gy(3, 3);
    Gy(0,0) = -1;  Gy(0,1) = -2;  Gy(0,2) = -1;
    Gy(1,0) =  0;  Gy(1,1) =  0;  Gy(1,2) =  0;
    Gy(2,0) =  1;  Gy(2,1) =  2;  Gy(2,2) =  1;

    const Matrix<Pixel>& srcMat = src.getMatrix();
    Image result(w, h);
    Matrix<Pixel>& dstMat = result.getMatrix();

    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            int sumX = 0, sumY = 0;

            // convolve 3×3 neighbourhood
            for (int ki = -1; ki <= 1; ki++) {
                for (int kj = -1; kj <= 1; kj++) {
                    int ni = i + ki;
                    int nj = j + kj;

                    // clamp to image bounds
                    if (ni < 0) ni = 0;
                    if (ni >= h) ni = h - 1;
                    if (nj < 0) nj = 0;
                    if (nj >= w) nj = w - 1;

                    // luminance of neighbour pixel
                    const Pixel& p = srcMat(ni, nj);
                    int lum = (int)(0.299 * p.r + 0.587 * p.g + 0.114 * p.b);

                    sumX += lum * Gx(ki + 1, kj + 1);
                    sumY += lum * Gy(ki + 1, kj + 1);
                }
            }

            // gradient magnitude, clamped to [0, 255]
            int mag = (int)sqrt((double)(sumX * sumX + sumY * sumY));
            if (mag > 255) mag = 255;

            unsigned char v = (unsigned char)mag;
            dstMat(i, j) = Pixel(v, v, v);
        }
    }
    return result;
}

// ================================================================
//  Nearest-Neighbor Scaling
//  percent: 50 → half, 100 → same, 200 → double
// ================================================================
Image ImageProcessor::scale(const Image& src, int percent) {
    int srcW = src.getWidth();
    int srcH = src.getHeight();

    int newW = srcW * percent / 100;
    int newH = srcH * percent / 100;
    if (newW < 1) newW = 1;
    if (newH < 1) newH = 1;

    Image result(newW, newH);

    const Matrix<Pixel>& srcMat = src.getMatrix();
    Matrix<Pixel>&       dstMat = result.getMatrix();

    for (int i = 0; i < newH; i++) {
        for (int j = 0; j < newW; j++) {
            int srcRow = i * srcH / newH;
            int srcCol = j * srcW / newW;

            // clamp (safety)
            if (srcRow >= srcH) srcRow = srcH - 1;
            if (srcCol >= srcW) srcCol = srcW - 1;

            dstMat(i, j) = srcMat(srcRow, srcCol);
        }
    }
    return result;
}

// ================================================================
//  Brightness / Contrast Adjustment
//  brightness: offset added to each channel (−255 to +255)
//  contrast:   multiplier applied to each channel (0.0 to 3.0)
//  Formula per channel:  new = clamp(contrast * old + brightness)
// ================================================================
Image ImageProcessor::adjustBrightnessContrast(const Image& src,
                                                int brightness, double contrast) {
    int w = src.getWidth();
    int h = src.getHeight();
    Image result(w, h);

    const Matrix<Pixel>& srcMat = src.getMatrix();
    Matrix<Pixel>&       dstMat = result.getMatrix();

    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            const Pixel& p = srcMat(i, j);

            // Apply contrast (scalar multiply) then brightness (offset)
            int r = (int)(contrast * p.r + brightness);
            int g = (int)(contrast * p.g + brightness);
            int b = (int)(contrast * p.b + brightness);

            // Clamp to [0, 255]
            if (r < 0) r = 0;  if (r > 255) r = 255;
            if (g < 0) g = 0;  if (g > 255) g = 255;
            if (b < 0) b = 0;  if (b > 255) b = 255;

            dstMat(i, j) = Pixel((unsigned char)r,
                                  (unsigned char)g,
                                  (unsigned char)b);
        }
    }
    return result;
}

// ================================================================
//  Image Sharpening — 3×3 convolution kernel
//
//  Sharpening kernel (using Matrix<int>):
//      0  -1   0
//     -1   5  -1
//      0  -1   0
//
//  This is the identity minus the Laplacian: enhances edges
//  while preserving the original brightness.
// ================================================================
Image ImageProcessor::sharpen(const Image& src) {
    int w = src.getWidth();
    int h = src.getHeight();

    // Build sharpening kernel using Matrix<int> (DSA requirement)
    Matrix<int> kernel(3, 3);
    kernel(0,0) =  0;  kernel(0,1) = -1;  kernel(0,2) =  0;
    kernel(1,0) = -1;  kernel(1,1) =  5;  kernel(1,2) = -1;
    kernel(2,0) =  0;  kernel(2,1) = -1;  kernel(2,2) =  0;

    const Matrix<Pixel>& srcMat = src.getMatrix();
    Image result(w, h);
    Matrix<Pixel>& dstMat = result.getMatrix();

    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            int sumR = 0, sumG = 0, sumB = 0;

            // Convolve 3×3 neighbourhood
            for (int ki = -1; ki <= 1; ki++) {
                for (int kj = -1; kj <= 1; kj++) {
                    int ni = i + ki;
                    int nj = j + kj;

                    // Clamp to image bounds
                    if (ni < 0) ni = 0;
                    if (ni >= h) ni = h - 1;
                    if (nj < 0) nj = 0;
                    if (nj >= w) nj = w - 1;

                    const Pixel& p = srcMat(ni, nj);
                    int kVal = kernel(ki + 1, kj + 1);

                    sumR += p.r * kVal;
                    sumG += p.g * kVal;
                    sumB += p.b * kVal;
                }
            }

            // Clamp results to [0, 255]
            if (sumR < 0) sumR = 0;  if (sumR > 255) sumR = 255;
            if (sumG < 0) sumG = 0;  if (sumG > 255) sumG = 255;
            if (sumB < 0) sumB = 0;  if (sumB > 255) sumB = 255;

            dstMat(i, j) = Pixel((unsigned char)sumR,
                                  (unsigned char)sumG,
                                  (unsigned char)sumB);
        }
    }
    return result;
}
