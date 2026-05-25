#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QComboBox>
#include <QStatusBar>
#include <QSlider>
#include <QSpinBox>
#include <QCheckBox>
#include <QTimer>
#include "Image.h"
#include "ImageProcessor.h"
#include "HuffmanCompressor.h"

/**
 * MainWindow — Qt6 GUI matching the Stitch "VisionEngine Pro" design.
 *
 * Layout:
 *   Left sidebar  (280px): operations + run buttons
 *   Central area:          2 image panels (Original / Processed)
 *   Right sidebar (260px): Compression Statistics panel
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void onOpenImage();
    void onSaveImage();
    void onGrayscale();
    void onEdgeDetect();
    void onScale();
    void onApplyAdjustments();
    void scheduleLivePreview();
    void onCompress();
    void onOpenDecompressDialog();

private:
    // ---- Image data ----
    Image originalImage;
    Image processedImage;
    HuffmanCompressor compressor;
    QString lastCompressedPath;

    // ---- Central image panels ----
    QLabel* lblOriginalCanvas;
    QLabel* lblProcessedCanvas;
    QLabel* lblOriginalTitle;
    QLabel* lblProcessedTitle;
    QLabel* lblOrigDim;
    QLabel* lblProcDim;

    // ---- Right sidebar: stats ----
    QLabel* lblOriginalSize;
    QLabel* lblCompressedSize;
    QLabel* lblCompressionRate;
    QProgressBar* progressBar;
    QLabel* lblStatusMessage;

    // ---- Left sidebar: controls ----
    QComboBox* cmbScalePreset;
    QSlider* sldBrightness;
    QSlider* sldContrast;
    QLabel* lblBrightVal;
    QLabel* lblContrastVal;
    QCheckBox* chkSharpen;
    QTimer* livePreviewTimer;

    // ---- Helpers ----
    void setupUi();
    void applyStitchStylesheet();
    void displayImage(const Image& img, QLabel* canvas, QLabel* dimLabel);
    void updateStats(long originalBytes, long compressedBytes);
    void clearStats();
};

#endif // MAINWINDOW_H
