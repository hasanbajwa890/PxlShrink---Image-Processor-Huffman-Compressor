#include <QApplication>
#include "MainWindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    // Set application metadata
    QApplication::setApplicationName("VisionEngine Pro");
    QApplication::setOrganizationName("Bahria University");
    QApplication::setApplicationVersion("2.4.0");

    MainWindow window;
    window.show();

    return app.exec();
}
