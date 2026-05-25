#include "DecompressDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QImage>
#include <QPixmap>
#include <QFrame>

// ================================================================
//  Constructor / Destructor
// ================================================================
DecompressDialog::DecompressDialog(QWidget* parent)
    : QDialog(parent) {
    setupUi();
    setWindowTitle("PxlShrink — Huffman Decompressor");
    resize(720, 600);
    setMinimumSize(500, 400);
}

DecompressDialog::~DecompressDialog() {}

// ================================================================
//  UI Construction
// ================================================================
void DecompressDialog::setupUi() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(14);

    // ---- Title ----
    QLabel* lblTitle = new QLabel("HUFFMAN DECOMPRESSOR");
    lblTitle->setObjectName("sectionLabel");
    lblTitle->setStyleSheet(
        "font-family: 'Consolas', monospace;"
        "font-size: 14px; font-weight: 700;"
        "color: #ADC6FF; letter-spacing: 3px;"
        "padding-bottom: 4px;"
    );
    mainLayout->addWidget(lblTitle);

    QFrame* sep = new QFrame;
    sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet("background-color: #2C2C2C; max-height: 1px;");
    mainLayout->addWidget(sep);

    // ---- Browse row ----
    QHBoxLayout* browseRow = new QHBoxLayout;
    QPushButton* btnBrowse = new QPushButton("  Browse .huf File...");
    btnBrowse->setObjectName("primaryButton");
    btnBrowse->setFixedHeight(38);
    connect(btnBrowse, &QPushButton::clicked, this, &DecompressDialog::onBrowseFile);
    browseRow->addWidget(btnBrowse);

    btnSave = new QPushButton("  Save as PPM");
    btnSave->setObjectName("opButton");
    btnSave->setFixedHeight(38);
    btnSave->setEnabled(false);
    connect(btnSave, &QPushButton::clicked, this, &DecompressDialog::onSaveImage);
    browseRow->addWidget(btnSave);

    mainLayout->addLayout(browseRow);

    // ---- File path label ----
    lblFilePath = new QLabel("No file selected");
    lblFilePath->setObjectName("dimLabel");
    lblFilePath->setWordWrap(true);
    mainLayout->addWidget(lblFilePath);

    // ---- Dimension info ----
    lblDimInfo = new QLabel("No image");
    lblDimInfo->setObjectName("dimLabel");
    mainLayout->addWidget(lblDimInfo);

    // ---- Image canvas ----
    lblCanvas = new QLabel;
    lblCanvas->setObjectName("imageCanvas");
    lblCanvas->setMinimumSize(200, 200);
    lblCanvas->setAlignment(Qt::AlignCenter);
    lblCanvas->setText("Browse a .huf file\nto decompress");
    lblCanvas->setScaledContents(false);
    lblCanvas->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    mainLayout->addWidget(lblCanvas, 1);

    // ---- Apply the same dark theme ----
    setStyleSheet(
        "QDialog {"
        "  background-color: #131313;"
        "  color: #E5E2E1;"
        "  font-family: 'Segoe UI', sans-serif;"
        "  font-size: 13px;"
        "}"
        "#primaryButton {"
        "  background-color: #007AFF;"
        "  color: #FFFFFF;"
        "  border: none;"
        "  padding: 10px 16px;"
        "  font-family: 'Consolas', monospace;"
        "  font-size: 12px;"
        "  font-weight: 600;"
        "  text-align: left;"
        "}"
        "#primaryButton:hover { background-color: #3395FF; }"
        "#primaryButton:pressed { background-color: #0055CC; }"
        "#opButton {"
        "  background-color: transparent;"
        "  color: #C1C6D7;"
        "  border: 1px solid #2C2C2C;"
        "  padding: 8px 12px;"
        "  font-family: 'Segoe UI', sans-serif;"
        "  font-size: 12px;"
        "  text-align: left;"
        "}"
        "#opButton:hover {"
        "  background-color: #252525;"
        "  border-color: #007AFF;"
        "  color: #FFFFFF;"
        "}"
        "#opButton:disabled {"
        "  color: #555;"
        "  border-color: #1E1E1E;"
        "}"
        "#imageCanvas {"
        "  background-color: #1E1E1E;"
        "  border: 1px solid #2C2C2C;"
        "  color: #414755;"
        "  font-family: 'Segoe UI', sans-serif;"
        "  font-size: 13px;"
        "}"
        "#dimLabel {"
        "  font-family: 'Consolas', monospace;"
        "  font-size: 11px;"
        "  color: #6B7080;"
        "}"
    );
}

// ================================================================
//  Slots
// ================================================================

void DecompressDialog::onBrowseFile() {
    QString path = QFileDialog::getOpenFileName(
        this, "Open Compressed File", "",
        "Huffman Files (*.huf);;All Files (*)");
    if (path.isEmpty()) return;

    lblFilePath->setText("File: " + path);

    int w = 0, h = 0;
    Matrix<Pixel>* result = compressor.decompress(path.toStdString().c_str(), w, h);
    if (!result) {
        QMessageBox::critical(this, "Error",
            "Decompression failed.\nInvalid or corrupted .huf file.");
        return;
    }

    // Build Image from decompressed matrix
    decompressedImage = Image(w, h);
    for (int i = 0; i < h; i++)
        for (int j = 0; j < w; j++)
            decompressedImage.setPixel(j, i, (*result)(i, j));

    delete result;

    displayImage(decompressedImage);
    lblDimInfo->setText(QString("%1 × %2 px · %3 KB")
        .arg(w).arg(h).arg(decompressedImage.getDataSize() / 1024));
    btnSave->setEnabled(true);
}

void DecompressDialog::onSaveImage() {
    if (!decompressedImage.isLoaded()) {
        QMessageBox::warning(this, "Warning", "No decompressed image to save.");
        return;
    }
    QString path = QFileDialog::getSaveFileName(
        this, "Save Decompressed Image", "decompressed.ppm",
        "PPM Files (*.ppm)");
    if (path.isEmpty()) return;

    if (decompressedImage.save(path.toStdString().c_str()))
        QMessageBox::information(this, "Saved",
            "Image saved successfully:\n" + path);
    else
        QMessageBox::critical(this, "Error", "Failed to save image.");
}

// ================================================================
//  Helpers
// ================================================================

void DecompressDialog::displayImage(const Image& img) {
    int w = img.getWidth();
    int h = img.getHeight();
    const Matrix<Pixel>& mat = img.getMatrix();

    QImage qimg(w, h, QImage::Format_RGB888);
    for (int i = 0; i < h; i++) {
        unsigned char* scanline = qimg.scanLine(i);
        for (int j = 0; j < w; j++) {
            const Pixel& p = mat(i, j);
            scanline[j * 3 + 0] = p.r;
            scanline[j * 3 + 1] = p.g;
            scanline[j * 3 + 2] = p.b;
        }
    }

    QPixmap pixmap = QPixmap::fromImage(qimg);
    lblCanvas->setPixmap(pixmap.scaled(lblCanvas->size(), Qt::KeepAspectRatio,
                                        Qt::SmoothTransformation));
}
