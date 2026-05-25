#ifndef IMAGE_PROCESSOR_H
#define IMAGE_PROCESSOR_H

#include "Image.h"
#include "Matrix.h"
#include "Pixel.h"

/**
 * ImageProcessor — Static methods for image transformations.
 * All operations are non-destructive: they return a new Image.
 * All convolutions use Matrix<int> for kernels (DSA requirement).
 */
class ImageProcessor {
public:
    /** RGB → Grayscale using luminance weights (ITU-R BT.601). */
    static Image toGrayscale(const Image& src);

    /** Sobel edge detection using two 3×3 Matrix<int> kernels. */
    static Image sobelEdgeDetect(const Image& src);

    /** Nearest-neighbor scaling by a percentage (50, 100, 200). */
    static Image scale(const Image& src, int percent);

    /** Brightness/Contrast adjustment using Matrix scalar operations. */
    static Image adjustBrightnessContrast(const Image& src, int brightness, double contrast);

    /** Image Sharpening using a 3×3 Matrix<int> kernel convolution. */
    static Image sharpen(const Image& src);
};

#endif // IMAGE_PROCESSOR_H
