#include <iostream>
#include "Matrix.h"
#include "Pixel.h"

// ---------------------------------------------------------------
// Tiny test helper — prints PASS / FAIL with a label.
// ---------------------------------------------------------------
static int totalTests  = 0;
static int passedTests = 0;

void check(bool condition, const char* label) {
    totalTests++;
    if (condition) {
        std::cout << "  [PASS] " << label << "\n";
        passedTests++;
    } else {
        std::cout << "  [FAIL] " << label << "\n";
    }
}

// ---------------------------------------------------------------
int main() {
    std::cout << "========================================\n";
    std::cout << "  Matrix<T> & Pixel  —  Test Suite\n";
    std::cout << "========================================\n\n";

    // -----------------------------------------------------------
    // 1. Basic construction & getters
    // -----------------------------------------------------------
    std::cout << "--- 1. Construction & Getters ---\n";
    {
        Matrix<int> m(3, 4);
        check(m.getRows() == 3, "rows == 3");
        check(m.getCols() == 4, "cols == 4");
        check(m(0, 0) == 0,    "default value == 0");
    }

    // -----------------------------------------------------------
    // 2. Element access via operator()(i, j)
    // -----------------------------------------------------------
    std::cout << "\n--- 2. Element Access ---\n";
    {
        Matrix<int> m(2, 2);
        m(0, 0) = 10;
        m(0, 1) = 20;
        m(1, 0) = 30;
        m(1, 1) = 40;
        check(m(0, 0) == 10, "m(0,0) == 10");
        check(m(0, 1) == 20, "m(0,1) == 20");
        check(m(1, 0) == 30, "m(1,0) == 30");
        check(m(1, 1) == 40, "m(1,1) == 40");
    }

    // -----------------------------------------------------------
    // 3. Copy Constructor — deep copy proof
    // -----------------------------------------------------------
    std::cout << "\n--- 3. Copy Constructor (deep copy) ---\n";
    {
        Matrix<int> original(2, 2);
        original(0, 0) = 1;
        original(1, 1) = 9;

        Matrix<int> copy(original);     // invoke copy ctor

        // Verify values were copied
        check(copy(0, 0) == 1, "copy(0,0) == original(0,0)");
        check(copy(1, 1) == 9, "copy(1,1) == original(1,1)");

        // Modify original — copy must NOT change
        original(0, 0) = 999;
        check(copy(0, 0) == 1, "copy unchanged after original modified (deep copy proof)");
    }

    // -----------------------------------------------------------
    // 4. Copy Assignment Operator — deep copy proof
    // -----------------------------------------------------------
    std::cout << "\n--- 4. Copy Assignment Operator ---\n";
    {
        Matrix<int> a(2, 2);
        a(0, 0) = 5;

        Matrix<int> b(1, 1);   // different dimensions
        b = a;                  // invoke operator=

        check(b.getRows() == 2, "assigned matrix rows == 2");
        check(b.getCols() == 2, "assigned matrix cols == 2");
        check(b(0, 0) == 5,    "b(0,0) == 5 after assignment");

        // Modify a — b must NOT change
        a(0, 0) = 888;
        check(b(0, 0) == 5, "b unchanged after a modified (deep copy proof)");

        // Self-assignment — must not crash
        b = b;
        check(b(0, 0) == 5, "self-assignment safe");
    }

    // -----------------------------------------------------------
    // 5. Addition
    // -----------------------------------------------------------
    std::cout << "\n--- 5. Addition ---\n";
    {
        Matrix<int> a(2, 2);
        Matrix<int> b(2, 2);
        a(0, 0) = 1; a(0, 1) = 2;
        a(1, 0) = 3; a(1, 1) = 4;
        b(0, 0) = 5; b(0, 1) = 6;
        b(1, 0) = 7; b(1, 1) = 8;

        Matrix<int> c = a + b;
        check(c(0, 0) ==  6, "1+5 == 6");
        check(c(0, 1) ==  8, "2+6 == 8");
        check(c(1, 0) == 10, "3+7 == 10");
        check(c(1, 1) == 12, "4+8 == 12");
    }

    // -----------------------------------------------------------
    // 6. Subtraction
    // -----------------------------------------------------------
    std::cout << "\n--- 6. Subtraction ---\n";
    {
        Matrix<int> a(2, 2);
        Matrix<int> b(2, 2);
        a(0, 0) = 10; a(0, 1) = 20;
        a(1, 0) = 30; a(1, 1) = 40;
        b(0, 0) =  1; b(0, 1) =  2;
        b(1, 0) =  3; b(1, 1) =  4;

        Matrix<int> c = a - b;
        check(c(0, 0) ==  9, "10-1 == 9");
        check(c(0, 1) == 18, "20-2 == 18");
        check(c(1, 0) == 27, "30-3 == 27");
        check(c(1, 1) == 36, "40-4 == 36");
    }

    // -----------------------------------------------------------
    // 7. Scalar Multiplication
    // -----------------------------------------------------------
    std::cout << "\n--- 7. Scalar Multiplication ---\n";
    {
        Matrix<int> a(2, 2);
        a(0, 0) = 1; a(0, 1) = 2;
        a(1, 0) = 3; a(1, 1) = 4;

        Matrix<int> c = a * 3;
        check(c(0, 0) ==  3, "1*3 == 3");
        check(c(0, 1) ==  6, "2*3 == 6");
        check(c(1, 0) ==  9, "3*3 == 9");
        check(c(1, 1) == 12, "4*3 == 12");
    }

    // -----------------------------------------------------------
    // 8. Matrix Multiplication
    // -----------------------------------------------------------
    std::cout << "\n--- 8. Matrix Multiplication ---\n";
    {
        // A = 2x3, B = 3x2  →  C = 2x2
        Matrix<int> a(2, 3);
        Matrix<int> b(3, 2);

        // A = | 1 2 3 |
        //     | 4 5 6 |
        a(0, 0) = 1; a(0, 1) = 2; a(0, 2) = 3;
        a(1, 0) = 4; a(1, 1) = 5; a(1, 2) = 6;

        // B = | 7  8  |
        //     | 9  10 |
        //     | 11 12 |
        b(0, 0) = 7;  b(0, 1) = 8;
        b(1, 0) = 9;  b(1, 1) = 10;
        b(2, 0) = 11; b(2, 1) = 12;

        // C = A * B
        // C[0][0] = 1*7 + 2*9  + 3*11 = 58
        // C[0][1] = 1*8 + 2*10 + 3*12 = 64
        // C[1][0] = 4*7 + 5*9  + 6*11 = 139
        // C[1][1] = 4*8 + 5*10 + 6*12 = 154
        Matrix<int> c = a * b;
        check(c.getRows() == 2,  "result rows == 2");
        check(c.getCols() == 2,  "result cols == 2");
        check(c(0, 0) ==  58,   "C[0][0] == 58");
        check(c(0, 1) ==  64,   "C[0][1] == 64");
        check(c(1, 0) == 139,   "C[1][0] == 139");
        check(c(1, 1) == 154,   "C[1][1] == 154");
    }

    // -----------------------------------------------------------
    // 9. Transpose
    // -----------------------------------------------------------
    std::cout << "\n--- 9. Transpose ---\n";
    {
        Matrix<int> m(2, 3);
        m(0, 0) = 1; m(0, 1) = 2; m(0, 2) = 3;
        m(1, 0) = 4; m(1, 1) = 5; m(1, 2) = 6;

        Matrix<int> t = m.transpose();
        check(t.getRows() == 3, "transposed rows == 3");
        check(t.getCols() == 2, "transposed cols == 2");
        check(t(0, 0) == 1, "t(0,0) == 1");
        check(t(1, 0) == 2, "t(1,0) == 2");
        check(t(2, 0) == 3, "t(2,0) == 3");
        check(t(0, 1) == 4, "t(0,1) == 4");
        check(t(1, 1) == 5, "t(1,1) == 5");
        check(t(2, 1) == 6, "t(2,1) == 6");
    }

    // -----------------------------------------------------------
    // 10. Matrix<Pixel> — construction & set/get
    // -----------------------------------------------------------
    std::cout << "\n--- 10. Matrix<Pixel> ---\n";
    {
        Matrix<Pixel> img(2, 2);

        // Default should be black (0, 0, 0)
        check(img(0, 0) == Pixel(0, 0, 0), "default pixel is black");

        // Set a red pixel
        img(0, 0) = Pixel(255, 0, 0);
        check(img(0, 0).r == 255, "red channel == 255");
        check(img(0, 0).g ==   0, "green channel == 0");
        check(img(0, 0).b ==   0, "blue channel == 0");

        // Set a white pixel
        img(1, 1) = Pixel(255, 255, 255);
        check(img(1, 1) == Pixel(255, 255, 255), "white pixel set correctly");

        // Deep copy of pixel matrix
        Matrix<Pixel> copy(img);
        img(0, 0) = Pixel(0, 0, 0);
        check(copy(0, 0) == Pixel(255, 0, 0), "pixel matrix deep copy intact");
    }

    // -----------------------------------------------------------
    // 11. Dimension mismatch error (should print to cerr)
    // -----------------------------------------------------------
    std::cout << "\n--- 11. Dimension Mismatch (expect cerr errors) ---\n";
    {
        Matrix<int> a(2, 2);
        Matrix<int> b(3, 3);
        Matrix<int> c = a + b;  // should print error
        check(c.getRows() == 0 && c.getCols() == 0, "mismatch returns 0x0 matrix");

        Matrix<int> d = a * b;  // should print error (2x2 * 3x3)
        check(d.getRows() == 0 && d.getCols() == 0, "multiply mismatch returns 0x0 matrix");
    }

    // -----------------------------------------------------------
    // Summary
    // -----------------------------------------------------------
    std::cout << "\n========================================\n";
    std::cout << "  Results: " << passedTests << " / " << totalTests << " passed\n";
    std::cout << "========================================\n";

    return (passedTests == totalTests) ? 0 : 1;
}
