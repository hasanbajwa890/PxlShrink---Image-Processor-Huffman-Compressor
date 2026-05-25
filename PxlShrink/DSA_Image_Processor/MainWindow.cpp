#include "MainWindow.h"
#include "DecompressDialog.h"
#include <QApplication>
#include <QMenuBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QFrame>
#include <QImage>
#include <QPixmap>
#include <QFont>
#include <QSplitter>
#include <QScrollArea>

// ================================================================
//  Constructor
// ================================================================
MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), lastCompressedPath("") {
    setupUi();
    applyStitchStylesheet();
    setWindowTitle("PxlShrink — DSA Image Processor");
    resize(1440, 900);

    // Live preview debounce timer (150ms delay)
    livePreviewTimer = new QTimer(this);
    livePreviewTimer->setSingleShot(true);
    livePreviewTimer->setInterval(150);
    connect(livePreviewTimer, &QTimer::timeout, this, &MainWindow::onApplyAdjustments);
}

MainWindow::~MainWindow() {}

// ================================================================
//  UI Construction — mirrors Stitch "VisionEngine Pro" layout
// ================================================================
void MainWindow::setupUi() {
    // ---- Menu Bar ----
    QMenuBar* menu = menuBar();
    QMenu* fileMenu = menu->addMenu("&File");
    fileMenu->addAction("&Open Image...", QKeySequence::Open, this, &MainWindow::onOpenImage);
    fileMenu->addAction("&Save Processed...", QKeySequence::Save, this, &MainWindow::onSaveImage);
    fileMenu->addSeparator();
    fileMenu->addAction("E&xit", QKeySequence::Quit, qApp, &QApplication::quit);

    // ============================================================
    //  LEFT SIDEBAR (280px) — Operation controls
    // ============================================================
    QWidget* leftSidebar = new QWidget;
    leftSidebar->setFixedWidth(280);
    leftSidebar->setObjectName("leftSidebar");
    QVBoxLayout* leftLayout = new QVBoxLayout(leftSidebar);
    leftLayout->setContentsMargins(16, 16, 16, 16);
    leftLayout->setSpacing(12);

    // Branding
    QLabel* lblBrand = new QLabel("PXLSHRINK");
    lblBrand->setObjectName("brandLabel");
    leftLayout->addWidget(lblBrand);

    // Separator
    QFrame* sep1 = new QFrame;
    sep1->setFrameShape(QFrame::HLine);
    sep1->setObjectName("separator");
    leftLayout->addWidget(sep1);

    // Section: FILTERS
    QLabel* lblFilters = new QLabel("FILTERS");
    lblFilters->setObjectName("sectionLabel");
    leftLayout->addWidget(lblFilters);

    QPushButton* btnGrayscale = new QPushButton("  Grayscale Conversion");
    btnGrayscale->setObjectName("opButton");
    connect(btnGrayscale, &QPushButton::clicked, this, &MainWindow::onGrayscale);
    leftLayout->addWidget(btnGrayscale);

    QPushButton* btnEdge = new QPushButton("  Sobel Edge Detection");
    btnEdge->setObjectName("opButton");
    connect(btnEdge, &QPushButton::clicked, this, &MainWindow::onEdgeDetect);
    leftLayout->addWidget(btnEdge);

    // Section: ADJUSTMENTS (combined Brightness/Contrast + Sharpening)
    QFrame* sepBC = new QFrame;
    sepBC->setFrameShape(QFrame::HLine);
    sepBC->setObjectName("separator");
    leftLayout->addWidget(sepBC);

    QLabel* lblBC = new QLabel("ADJUSTMENTS");
    lblBC->setObjectName("sectionLabel");
    leftLayout->addWidget(lblBC);

    // Brightness slider: -100 to +100
    QHBoxLayout* brightRow = new QHBoxLayout;
    QLabel* lblBright = new QLabel("Bright:");
    lblBright->setObjectName("dimLabel");
    sldBrightness = new QSlider(Qt::Horizontal);
    sldBrightness->setRange(-100, 100);
    sldBrightness->setValue(0);
    sldBrightness->setObjectName("adjustSlider");
    lblBrightVal = new QLabel("0");
    lblBrightVal->setObjectName("dimLabel");
    lblBrightVal->setFixedWidth(30);
    connect(sldBrightness, &QSlider::valueChanged, [this](int v) {
        lblBrightVal->setText(QString::number(v));
        scheduleLivePreview();
    });
    brightRow->addWidget(lblBright);
    brightRow->addWidget(sldBrightness, 1);
    brightRow->addWidget(lblBrightVal);
    leftLayout->addLayout(brightRow);

    // Contrast slider: 50 to 200 (represents 0.5x to 2.0x)
    QHBoxLayout* contRow = new QHBoxLayout;
    QLabel* lblCont = new QLabel("Contrast:");
    lblCont->setObjectName("dimLabel");
    sldContrast = new QSlider(Qt::Horizontal);
    sldContrast->setRange(50, 200);
    sldContrast->setValue(100);
    sldContrast->setObjectName("adjustSlider");
    lblContrastVal = new QLabel("1.0");
    lblContrastVal->setObjectName("dimLabel");
    lblContrastVal->setFixedWidth(30);
    connect(sldContrast, &QSlider::valueChanged, [this](int v) {
        lblContrastVal->setText(QString::number(v / 100.0, 'f', 1));
        scheduleLivePreview();
    });
    contRow->addWidget(lblCont);
    contRow->addWidget(sldContrast, 1);
    contRow->addWidget(lblContrastVal);
    leftLayout->addLayout(contRow);

    // Sharpen checkbox
    chkSharpen = new QCheckBox("  Apply Sharpening");
    chkSharpen->setObjectName("sharpenCheck");
    connect(chkSharpen, &QCheckBox::toggled, [this](bool) {
        scheduleLivePreview();
    });
    leftLayout->addWidget(chkSharpen);

    // Section: GEOMETRY
    QFrame* sep2 = new QFrame;
    sep2->setFrameShape(QFrame::HLine);
    sep2->setObjectName("separator");
    leftLayout->addWidget(sep2);

    QLabel* lblGeom = new QLabel("GEOMETRY");
    lblGeom->setObjectName("sectionLabel");
    leftLayout->addWidget(lblGeom);

    cmbScalePreset = new QComboBox;
    cmbScalePreset->addItem("50%",  50);
    cmbScalePreset->addItem("100%", 100);
    cmbScalePreset->addItem("200%", 200);
    cmbScalePreset->setCurrentIndex(1);
    cmbScalePreset->setObjectName("scaleCombo");
    connect(cmbScalePreset, &QComboBox::currentIndexChanged, this, &MainWindow::onScale);
    leftLayout->addWidget(cmbScalePreset);

    // Section: COMPRESSION
    QFrame* sep3 = new QFrame;
    sep3->setFrameShape(QFrame::HLine);
    sep3->setObjectName("separator");
    leftLayout->addWidget(sep3);

    QLabel* lblComp = new QLabel("COMPRESSION");
    lblComp->setObjectName("sectionLabel");
    leftLayout->addWidget(lblComp);

    QPushButton* btnCompress = new QPushButton("  Huffman Compress");
    btnCompress->setObjectName("primaryButton");
    connect(btnCompress, &QPushButton::clicked, this, &MainWindow::onCompress);
    leftLayout->addWidget(btnCompress);

    QPushButton* btnDecompress = new QPushButton("  Huffman Decompress");
    btnDecompress->setObjectName("opButton");
    connect(btnDecompress, &QPushButton::clicked, this, &MainWindow::onOpenDecompressDialog);
    leftLayout->addWidget(btnDecompress);

    leftLayout->addStretch();

    // Footer status
    lblStatusMessage = new QLabel("Ready — load an image to begin.");
    lblStatusMessage->setObjectName("statusLabel");
    lblStatusMessage->setWordWrap(true);
    leftLayout->addWidget(lblStatusMessage);

    // ============================================================
    //  CENTRAL AREA — Two image canvases side-by-side
    // ============================================================
    QWidget* centralWidget = new QWidget;
    QHBoxLayout* canvasLayout = new QHBoxLayout(centralWidget);
    canvasLayout->setSpacing(8);
    canvasLayout->setContentsMargins(8, 8, 8, 8);

    auto makeCanvasPanel = [&](const QString& title, QLabel*& canvas,
                               QLabel*& titleLbl, QLabel*& dimLbl) {
        QWidget* panel = new QWidget;
        panel->setObjectName("canvasPanel");
        panel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        QVBoxLayout* vbox = new QVBoxLayout(panel);
        vbox->setContentsMargins(12, 12, 12, 12);
        vbox->setSpacing(8);

        titleLbl = new QLabel(title);
        titleLbl->setObjectName("canvasTitle");

        dimLbl = new QLabel("No image");
        dimLbl->setObjectName("dimLabel");

        canvas = new QLabel;
        canvas->setObjectName("imageCanvas");
        canvas->setMinimumSize(120, 100);
        canvas->setAlignment(Qt::AlignCenter);
        canvas->setText("Drop or open\nan image");
        canvas->setScaledContents(false);
        canvas->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

        vbox->addWidget(titleLbl);
        vbox->addWidget(dimLbl);
        vbox->addWidget(canvas, 1);

        return panel;
    };

    QWidget* p1 = makeCanvasPanel("ORIGINAL",      lblOriginalCanvas,
                                   lblOriginalTitle, lblOrigDim);
    QWidget* p2 = makeCanvasPanel("PROCESSED",      lblProcessedCanvas,
                                   lblProcessedTitle, lblProcDim);

    canvasLayout->addWidget(p1, 1);
    canvasLayout->addWidget(p2, 1);

    // ============================================================
    //  RIGHT SIDEBAR — Compression Statistics
    // ============================================================
    QWidget* rightSidebar = new QWidget;
    rightSidebar->setMinimumWidth(200);
    rightSidebar->setMaximumWidth(300);
    rightSidebar->setObjectName("rightSidebar");
    rightSidebar->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    QVBoxLayout* rightLayout = new QVBoxLayout(rightSidebar);
    rightLayout->setContentsMargins(16, 16, 16, 16);
    rightLayout->setSpacing(12);

    QLabel* lblStatsTitle = new QLabel("COMPRESSION STATISTICS");
    lblStatsTitle->setObjectName("sectionLabel");
    rightLayout->addWidget(lblStatsTitle);

    // Size tracking group
    QLabel* lblSizeHdr = new QLabel("SIZE TRACKING");
    lblSizeHdr->setObjectName("subSectionLabel");
    rightLayout->addWidget(lblSizeHdr);

    lblOriginalSize = new QLabel("Original: — ");
    lblOriginalSize->setObjectName("statValue");
    lblOriginalSize->setWordWrap(true);
    rightLayout->addWidget(lblOriginalSize);

    lblCompressedSize = new QLabel("Compressed: — ");
    lblCompressedSize->setObjectName("statValue");
    lblCompressedSize->setWordWrap(true);
    rightLayout->addWidget(lblCompressedSize);

    QFrame* sep4 = new QFrame;
    sep4->setFrameShape(QFrame::HLine);
    sep4->setObjectName("separator");
    rightLayout->addWidget(sep4);

    // Compression rate
    QLabel* lblRateHdr = new QLabel("COMPRESSION RATE");
    lblRateHdr->setObjectName("subSectionLabel");
    rightLayout->addWidget(lblRateHdr);

    lblCompressionRate = new QLabel("0.0 %");
    lblCompressionRate->setObjectName("rateValue");
    rightLayout->addWidget(lblCompressionRate);

    progressBar = new QProgressBar;
    progressBar->setRange(0, 100);
    progressBar->setValue(0);
    progressBar->setTextVisible(false);
    progressBar->setObjectName("compressionProgress");
    rightLayout->addWidget(progressBar);

    rightLayout->addStretch();

    // ============================================================
    //  Main layout assembly — QSplitter for responsive resizing
    // ============================================================
    QSplitter* splitter = new QSplitter(Qt::Horizontal);
    splitter->setHandleWidth(1);
    splitter->setChildrenCollapsible(false);
    splitter->addWidget(centralWidget);
    splitter->addWidget(rightSidebar);
    splitter->setStretchFactor(0, 1);   // central area gets extra space
    splitter->setStretchFactor(1, 0);   // right sidebar stays compact

    QWidget* mainWidget = new QWidget;
    QHBoxLayout* mainLayout = new QHBoxLayout(mainWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(leftSidebar);
    mainLayout->addWidget(splitter, 1);

    setCentralWidget(mainWidget);

    // Status bar
    statusBar()->showMessage("PxlShrink — Bahria University DSA Project");
}

// ================================================================
//  Stitch Design DNA → Qt Stylesheet
// ================================================================
void MainWindow::applyStitchStylesheet() {
    QString qss = R"(
        /* ---- Global ---- */
        QMainWindow, QWidget {
            background-color: #131313;
            color: #E5E2E1;
            font-family: "Segoe UI", sans-serif;
            font-size: 13px;
        }

        /* ---- Menu bar ---- */
        QMenuBar {
            background-color: #1E1E1E;
            color: #E5E2E1;
            border-bottom: 1px solid #2C2C2C;
            padding: 4px;
            font-family: "Segoe UI", sans-serif;
        }
        QMenuBar::item:selected { background-color: #2A2A2A; }
        QMenu {
            background-color: #252525;
            color: #E5E2E1;
            border: 1px solid #2C2C2C;
        }
        QMenu::item:selected { background-color: #007AFF; }

        /* ---- Status bar ---- */
        QStatusBar {
            background-color: #1E1E1E;
            color: #8B90A0;
            border-top: 1px solid #2C2C2C;
            font-family: "Consolas", monospace;
            font-size: 11px;
        }

        /* ---- Left sidebar ---- */
        #leftSidebar {
            background-color: #1E1E1E;
            border-right: 1px solid #2C2C2C;
        }

        /* ---- Right sidebar ---- */
        #rightSidebar {
            background-color: #1E1E1E;
            border-left: 1px solid #2C2C2C;
        }

        /* ---- Brand label ---- */
        #brandLabel {
            font-family: "Segoe UI", sans-serif;
            font-size: 18px;
            font-weight: 600;
            color: #ADC6FF;
            letter-spacing: 2px;
        }

        /* ---- Section labels ---- */
        #sectionLabel {
            font-family: "Consolas", monospace;
            font-size: 10px;
            font-weight: 700;
            color: #8B90A0;
            letter-spacing: 3px;
            padding-top: 4px;
        }
        #subSectionLabel {
            font-family: "Consolas", monospace;
            font-size: 10px;
            font-weight: 600;
            color: #6B7080;
            letter-spacing: 2px;
        }

        /* ---- Separator lines ---- */
        #separator {
            color: #2C2C2C;
            max-height: 1px;
            background-color: #2C2C2C;
        }

        /* ---- Operation buttons — clean flat style, no border ---- */
        #opButton {
            background-color: transparent;
            color: #C1C6D7;
            border: none;
            padding: 8px 12px;
            font-family: "Segoe UI", sans-serif;
            font-size: 12px;
            text-align: left;
        }
        #opButton:hover {
            background-color: #252525;
            color: #FFFFFF;
        }
        #opButton:pressed {
            background-color: #007AFF;
            color: #FFFFFF;
        }

        /* ---- Primary action button ---- */
        #primaryButton {
            background-color: #007AFF;
            color: #FFFFFF;
            border: none;
            padding: 10px 16px;
            font-family: "Consolas", monospace;
            font-size: 12px;
            font-weight: 600;
            text-align: left;
        }
        #primaryButton:hover {
            background-color: #3395FF;
        }
        #primaryButton:pressed {
            background-color: #0055CC;
        }

        /* ---- Sliders — clean flat groove + handle ---- */
        #adjustSlider {
            background: transparent;
            border: none;
        }
        #adjustSlider::groove:horizontal {
            background: #2C2C2C;
            height: 4px;
            border-radius: 2px;
        }
        #adjustSlider::handle:horizontal {
            background: #007AFF;
            width: 14px;
            height: 14px;
            margin: -5px 0;
            border-radius: 7px;
            border: none;
        }
        #adjustSlider::handle:horizontal:hover {
            background: #3395FF;
        }

        /* ---- Scale combo box — minimal border ---- */
        #scaleCombo {
            background-color: #1A1A1A;
            color: #E5E2E1;
            border: none;
            border-bottom: 1px solid #2C2C2C;
            padding: 6px 10px;
            font-family: "Consolas", monospace;
            font-size: 12px;
        }
        #scaleCombo:focus { border-bottom-color: #007AFF; }
        #scaleCombo QAbstractItemView {
            background-color: #252525;
            color: #E5E2E1;
            selection-background-color: #007AFF;
            border: 1px solid #2C2C2C;
        }

        /* ---- Canvas panels ---- */
        #canvasPanel {
            background-color: #1E1E1E;
            border: 1px solid #2C2C2C;
        }
        #canvasTitle {
            font-family: "Consolas", monospace;
            font-size: 10px;
            font-weight: 700;
            color: #8B90A0;
            letter-spacing: 3px;
        }
        #dimLabel {
            font-family: "Consolas", monospace;
            font-size: 11px;
            color: #6B7080;
            border: none;
        }
        #imageCanvas {
            background-color: #131313;
            border: 1px solid #2C2C2C;
            color: #414755;
            font-family: "Segoe UI", sans-serif;
            font-size: 13px;
        }

        /* ---- Stats values ---- */
        #statValue {
            font-family: "Consolas", monospace;
            font-size: 13px;
            color: #E5E2E1;
            padding: 4px 0px;
        }
        #rateValue {
            font-family: "Segoe UI", sans-serif;
            font-size: 28px;
            font-weight: 600;
            color: #41E4C0;
            padding: 4px 0px;
        }

        /* ---- Progress bar ---- */
        #compressionProgress {
            background-color: #252525;
            border: none;
            max-height: 6px;
        }
        #compressionProgress::chunk {
            background-color: #41E4C0;
        }

        /* ---- Checkbox — clean flat style ---- */
        #sharpenCheck {
            font-family: "Segoe UI", sans-serif;
            font-size: 12px;
            color: #C1C6D7;
            spacing: 8px;
            padding: 4px 0px;
        }
        #sharpenCheck::indicator {
            width: 16px;
            height: 16px;
            border: 1px solid #3A3A3A;
            border-radius: 3px;
            background-color: transparent;
        }
        #sharpenCheck::indicator:checked {
            background-color: #007AFF;
            border-color: #007AFF;
        }
        #sharpenCheck::indicator:hover {
            border-color: #007AFF;
        }

        /* ---- Status label ---- */
        #statusLabel {
            font-family: "Segoe UI", sans-serif;
            font-size: 11px;
            color: #6B7080;
            padding: 8px 0px;
        }
    )";
    qApp->setStyleSheet(qss);
}

// ================================================================
//  Slot Implementations
// ================================================================

void MainWindow::onOpenImage() {
    QString path = QFileDialog::getOpenFileName(
        this, "Open PPM Image", "", "PPM Files (*.ppm);;All Files (*)");
    if (path.isEmpty()) return;

    if (!originalImage.load(path.toStdString().c_str())) {
        QMessageBox::critical(this, "Error", "Failed to load PPM image.\nEnsure it is a valid P6 file.");
        return;
    }

    displayImage(originalImage, lblOriginalCanvas, lblOrigDim);

    // Clear processed
    lblProcessedCanvas->clear();
    lblProcessedCanvas->setText("Apply a filter");
    lblProcDim->setText("No image");
    clearStats();

    lblStatusMessage->setText(QString("Loaded: %1\n%2 × %3 px")
        .arg(path.split("/").last().split("\\").last())
        .arg(originalImage.getWidth())
        .arg(originalImage.getHeight()));
    statusBar()->showMessage("Image loaded successfully.");
}

void MainWindow::onSaveImage() {
    if (!processedImage.isLoaded()) {
        QMessageBox::warning(this, "Warning", "No processed image to save.");
        return;
    }
    QString path = QFileDialog::getSaveFileName(
        this, "Save Processed Image", "output.ppm", "PPM Files (*.ppm)");
    if (path.isEmpty()) return;

    if (processedImage.save(path.toStdString().c_str()))
        statusBar()->showMessage("Image saved: " + path);
    else
        QMessageBox::critical(this, "Error", "Failed to save image.");
}

void MainWindow::onGrayscale() {
    if (!originalImage.isLoaded()) {
        QMessageBox::warning(this, "Warning", "Load an image first.");
        return;
    }
    processedImage = ImageProcessor::toGrayscale(originalImage);
    displayImage(processedImage, lblProcessedCanvas, lblProcDim);
    lblStatusMessage->setText("Applied: Grayscale (BT.601)");
    statusBar()->showMessage("Grayscale conversion complete.");
}

void MainWindow::onEdgeDetect() {
    if (!originalImage.isLoaded()) {
        QMessageBox::warning(this, "Warning", "Load an image first.");
        return;
    }
    processedImage = ImageProcessor::sobelEdgeDetect(originalImage);
    displayImage(processedImage, lblProcessedCanvas, lblProcDim);
    lblStatusMessage->setText("Applied: Sobel Edge Detection\n3×3 Gx/Gy kernels");
    statusBar()->showMessage("Sobel edge detection complete.");
}

void MainWindow::onScale() {
    if (!originalImage.isLoaded()) {
        QMessageBox::warning(this, "Warning", "Load an image first.");
        return;
    }
    int percent = cmbScalePreset->currentData().toInt();
    processedImage = ImageProcessor::scale(originalImage, percent);
    displayImage(processedImage, lblProcessedCanvas, lblProcDim);
    lblStatusMessage->setText(QString("Applied: Scale %1%\nNearest-neighbor").arg(percent));
    statusBar()->showMessage(QString("Scaled to %1%.").arg(percent));
}

void MainWindow::onApplyAdjustments() {
    if (!originalImage.isLoaded()) {
        QMessageBox::warning(this, "Warning", "Load an image first.");
        return;
    }

    int brightness = sldBrightness->value();
    double contrast = sldContrast->value() / 100.0;
    bool sharpen = chkSharpen->isChecked();

    // Step 1: Apply brightness/contrast to the original image
    processedImage = ImageProcessor::adjustBrightnessContrast(
        originalImage, brightness, contrast);

    // Step 2: If sharpening is enabled, apply it on top of the B/C result
    if (sharpen) {
        processedImage = ImageProcessor::sharpen(processedImage);
    }

    displayImage(processedImage, lblProcessedCanvas, lblProcDim);

    QString statusText;
    if (sharpen) {
        statusText = QString("Applied: B=%1 C=%2 + Sharpening\nCombined adjustment pipeline")
            .arg(brightness).arg(contrast, 0, 'f', 1);
    } else {
        statusText = QString("Applied: Brightness %1, Contrast %2\nScalar matrix ops")
            .arg(brightness).arg(contrast, 0, 'f', 1);
    }
    lblStatusMessage->setText(statusText);
    statusBar()->showMessage("Adjustments applied.");
}

void MainWindow::onCompress() {
    // Compress whichever image is latest (processed > original)
    const Image& target = processedImage.isLoaded() ? processedImage : originalImage;
    if (!target.isLoaded()) {
        QMessageBox::warning(this, "Warning", "No image to compress.");
        return;
    }

    QString path = QFileDialog::getSaveFileName(
        this, "Save Compressed File", "compressed.huf", "Huffman Files (*.huf)");
    if (path.isEmpty()) return;

    long compSize = compressor.compress(target.getMatrix(),
                                         path.toStdString().c_str());
    if (compSize <= 0) {
        QMessageBox::critical(this, "Error", "Compression failed.");
        return;
    }

    lastCompressedPath = path;
    long origSize = target.getDataSize();
    updateStats(origSize, compSize);

    lblStatusMessage->setText(QString("Compressed to: %1\nHuffman coding (adaptive quantization)")
        .arg(path.split("/").last().split("\\").last()));
    statusBar()->showMessage("Huffman compression complete.");
}

void MainWindow::onOpenDecompressDialog() {
    DecompressDialog* dlg = new DecompressDialog(this);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->show();
}

// ================================================================
//  Helpers
// ================================================================

void MainWindow::displayImage(const Image& img, QLabel* canvas, QLabel* dimLabel) {
    int w = img.getWidth();
    int h = img.getHeight();
    const Matrix<Pixel>& mat = img.getMatrix();

    // Build QImage from Matrix<Pixel>
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
    // Scale to fit canvas while preserving aspect ratio
    canvas->setPixmap(pixmap.scaled(canvas->size(), Qt::KeepAspectRatio,
                                     Qt::SmoothTransformation));
    dimLabel->setText(QString("%1 × %2 px · %3 KB")
        .arg(w).arg(h).arg(img.getDataSize() / 1024));
}

void MainWindow::updateStats(long originalBytes, long compressedBytes) {
    double rate = (1.0 - (double)compressedBytes / (double)originalBytes) * 100.0;
    if (rate < 0) rate = 0;

    lblOriginalSize->setText(QString("Original:   %1 KB  (%2 B)")
        .arg(originalBytes / 1024).arg(originalBytes));
    lblCompressedSize->setText(QString("Compressed: %1 KB  (%2 B)")
        .arg(compressedBytes / 1024).arg(compressedBytes));
    lblCompressionRate->setText(QString("%1 %").arg(rate, 0, 'f', 1));
    progressBar->setValue((int)rate);
}

void MainWindow::clearStats() {
    lblOriginalSize->setText("Original: — ");
    lblCompressedSize->setText("Compressed: — ");
    lblCompressionRate->setText("0.0 %");
    progressBar->setValue(0);
}

void MainWindow::scheduleLivePreview() {
    if (originalImage.isLoaded()) {
        livePreviewTimer->start();   // restart the 150ms debounce
    }
}
