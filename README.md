# 🎨 PxlShrink — C++ Image Processor & Huffman Compressor

PxlShrink is a high-performance interactive desktop application built in **C++** and **Qt** for low-level image processing and Huffman-based lossless compression. 

To demonstrate strong computer science and software engineering fundamentals, this project implements all core Data Structures and Algorithms **entirely from scratch**—with zero dependencies on external image processing frameworks (like OpenCV) or standard helper containers (like `std::vector` or `std::priority_queue`).

---
## 🚀 Key Features
* **100% Custom Data Structures:** Manual heap memory management featuring a custom template-based 2D `Matrix` (fully compliant with the C++ Rule of Three), custom dynamic `Min-Heap`, and `Huffman Tree`.
* **Lossless Image Compression:** A custom binary compiler format (`.huf`) utilizing a greedy Huffman algorithm to serialize, encode, pack bits, and reconstruct images with **pixel-perfect lossless recovery**.
* **2D Convolution Filters:** Real-time spatial operations including **Sobel Edge Detection** (using horizontal and vertical $3 \times 3$ kernels) and BT.601 luminance-weighted **Grayscale Conversion**.
* **Nearest-Neighbor Scaling:** Dynamic geometric scaling (50%, 100%, 200%) preserving crisp edges.
* **Premium User Interface:** A responsive, modern multi-panel dark-themed layout styled and designed using **Stitch AI** featuring canvas synchronization and live compression statistics.
* **Robust Toolchain Launcher:** Dynamic self-healing build system launcher (`run.bat`) that auto-detects path relocation, resolves environment variables, and bypasses local Qt licensing limits.
