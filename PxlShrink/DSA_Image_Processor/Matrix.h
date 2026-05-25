#ifndef MATRIX_H
#define MATRIX_H

#include <iostream>

/**
 * Matrix<T> — A generic 2D matrix using dynamic memory allocation.
 *
 * Storage layout: row-major via T** (array of row pointers).
 *   data[i]    → pointer to row i  (allocated with new T[cols])
 *   data[i][j] → element at row i, column j
 *
 * Memory contract (Rule of Three):
 *   Constructor  → allocates
 *   Destructor   → deallocates
 *   Copy ctor    → deep copies   (prevents double-free)
 *   operator=    → deep copies   (prevents double-free)
 */
template <typename T>
class Matrix {
private:
    int rows;
    int cols;
    T** data;

    // ---------------------------------------------------------------
    // Helper: allocate the 2D array and zero-initialise every cell.
    // Used by the constructor and copy helpers to avoid code duplication.
    // ---------------------------------------------------------------
    void allocate(int r, int c) {
        rows = r;
        cols = c;
        data = new T*[rows];          // array of row pointers
        for (int i = 0; i < rows; i++) {
            data[i] = new T[cols];    // one row
            for (int j = 0; j < cols; j++) {
                data[i][j] = T();     // value-initialise (0 for numeric, black for Pixel)
            }
        }
    }

    // ---------------------------------------------------------------
    // Helper: free all heap memory owned by this matrix.
    // ---------------------------------------------------------------
    void deallocate() {
        if (data != nullptr) {
            for (int i = 0; i < rows; i++) {
                delete[] data[i];     // free each row
            }
            delete[] data;            // free the row-pointer array
            data = nullptr;
        }
    }

public:
    // ===============================================================
    //  Constructor
    // ===============================================================
    /**
     * Creates an r × c matrix.  Every cell is value-initialised:
     *   - numeric types → 0
     *   - Pixel         → (0, 0, 0)  (black)
     */
    Matrix(int r, int c) : rows(0), cols(0), data(nullptr) {
        allocate(r, c);
    }

    // ===============================================================
    //  Destructor
    // ===============================================================
    /**
     * Frees every row, then the row-pointer array.
     * Called automatically when the object goes out of scope.
     */
    ~Matrix() {
        deallocate();
    }

    // ===============================================================
    //  Copy Constructor  (deep copy)
    // ===============================================================
    /**
     * Allocates fresh memory and copies every element from `other`.
     * After this, `this` and `other` own completely separate data.
     */
    Matrix(const Matrix<T>& other) : rows(0), cols(0), data(nullptr) {
        allocate(other.rows, other.cols);
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                data[i][j] = other.data[i][j];
            }
        }
    }

    // ===============================================================
    //  Copy Assignment Operator  (deep copy)
    // ===============================================================
    /**
     * 1. Guard against self-assignment.
     * 2. Free existing memory.
     * 3. Allocate + deep-copy from `other`.
     */
    Matrix<T>& operator=(const Matrix<T>& other) {
        if (this == &other) return *this;   // self-assignment guard

        deallocate();                       // free old data

        allocate(other.rows, other.cols);   // allocate new
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                data[i][j] = other.data[i][j];
            }
        }
        return *this;
    }

    // ===============================================================
    //  Element Access — operator()(i, j)
    // ===============================================================
    /** Read-write access.  No bounds checking (for speed). */
    T& operator()(int i, int j) {
        return data[i][j];
    }

    /** Read-only access for const matrices. */
    const T& operator()(int i, int j) const {
        return data[i][j];
    }

    // ===============================================================
    //  Getters
    // ===============================================================
    int getRows() const { return rows; }
    int getCols() const { return cols; }

    // ===============================================================
    //  Matrix Addition
    // ===============================================================
    Matrix<T> operator+(const Matrix<T>& other) const {
        if (rows != other.rows || cols != other.cols) {
            std::cerr << "Error [operator+]: dimension mismatch ("
                      << rows << "x" << cols << " vs "
                      << other.rows << "x" << other.cols << ")\n";
            return Matrix<T>(0, 0);
        }
        Matrix<T> result(rows, cols);
        for (int i = 0; i < rows; i++)
            for (int j = 0; j < cols; j++)
                result.data[i][j] = data[i][j] + other.data[i][j];
        return result;
    }

    // ===============================================================
    //  Matrix Subtraction
    // ===============================================================
    Matrix<T> operator-(const Matrix<T>& other) const {
        if (rows != other.rows || cols != other.cols) {
            std::cerr << "Error [operator-]: dimension mismatch ("
                      << rows << "x" << cols << " vs "
                      << other.rows << "x" << other.cols << ")\n";
            return Matrix<T>(0, 0);
        }
        Matrix<T> result(rows, cols);
        for (int i = 0; i < rows; i++)
            for (int j = 0; j < cols; j++)
                result.data[i][j] = data[i][j] - other.data[i][j];
        return result;
    }

    // ===============================================================
    //  Scalar Multiplication
    // ===============================================================
    Matrix<T> operator*(T scalar) const {
        Matrix<T> result(rows, cols);
        for (int i = 0; i < rows; i++)
            for (int j = 0; j < cols; j++)
                result.data[i][j] = data[i][j] * scalar;
        return result;
    }

    // ===============================================================
    //  Matrix Multiplication
    // ===============================================================
    /**
     * Standard O(n³) multiplication.
     * Requires: this->cols == other.rows
     * Result dimensions: this->rows × other.cols
     */
    Matrix<T> operator*(const Matrix<T>& other) const {
        if (cols != other.rows) {
            std::cerr << "Error [operator* matrix]: dimension mismatch ("
                      << rows << "x" << cols << " * "
                      << other.rows << "x" << other.cols << ")\n";
            return Matrix<T>(0, 0);
        }
        Matrix<T> result(rows, other.cols);
        for (int i = 0; i < rows; i++)
            for (int j = 0; j < other.cols; j++)
                for (int k = 0; k < cols; k++)
                    result.data[i][j] = result.data[i][j] + data[i][k] * other.data[k][j];
        return result;
    }

    // ===============================================================
    //  Transpose
    // ===============================================================
    /** Returns a new cols × rows matrix with rows/cols swapped. */
    Matrix<T> transpose() const {
        Matrix<T> result(cols, rows);
        for (int i = 0; i < rows; i++)
            for (int j = 0; j < cols; j++)
                result.data[j][i] = data[i][j];
        return result;
    }
};

#endif // MATRIX_H
