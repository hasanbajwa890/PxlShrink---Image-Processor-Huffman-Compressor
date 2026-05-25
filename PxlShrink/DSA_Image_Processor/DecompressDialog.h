#ifndef DECOMPRESS_DIALOG_H
#define DECOMPRESS_DIALOG_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include "Image.h"
#include "HuffmanCompressor.h"

/**
 * DecompressDialog — Standalone dialog for decompressing .huf files.
 *
 * Opens independently of the main window. The user browses for a
 * .huf file, the dialog decompresses it and displays the result.
 * The decompressed image can be saved as a PPM file.
 */
class DecompressDialog : public QDialog {
    Q_OBJECT

public:
    explicit DecompressDialog(QWidget* parent = nullptr);
    ~DecompressDialog();

private slots:
    void onBrowseFile();
    void onSaveImage();

private:
    HuffmanCompressor compressor;
    Image decompressedImage;

    // UI elements
    QLabel* lblCanvas;
    QLabel* lblDimInfo;
    QLabel* lblFilePath;
    QPushButton* btnSave;

    void setupUi();
    void displayImage(const Image& img);
};

#endif // DECOMPRESS_DIALOG_H
