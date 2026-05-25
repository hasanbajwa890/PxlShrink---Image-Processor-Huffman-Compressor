// generate_test_ppm.cpp — Creates a sample 128x128 PPM P6 test image
// Compile: g++ generate_test_ppm.cpp -o generate_test_ppm && ./generate_test_ppm
#include <fstream>

int main() {
    const int W = 128, H = 128;
    std::ofstream f("test_image.ppm", std::ios::binary);
    f << "P6\n" << W << " " << H << "\n255\n";

    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            unsigned char r = (unsigned char)(x * 2);       // red gradient L→R
            unsigned char g = (unsigned char)(y * 2);       // green gradient T→B
            unsigned char b = (unsigned char)(128);          // constant blue
            f.put((char)r);
            f.put((char)g);
            f.put((char)b);
        }
    }
    f.close();
    return 0;
}
