#include <QApplication>
#include <QCoreApplication>
#include "browserwindow.h"

int main(int argc, char *argv[]) {
    // Chromium bayrakları ve donanım ivmelendirmesi
    qputenv("QTWEBENGINE_CHROMIUM_FLAGS", "--enable-gpu-rasterization --enable-zero-copy --enable-features=WebAssembly");

    QApplication app(argc, argv);

    app.setApplicationName("Srowser");
    app.setOrganizationName("SrowserCorp");

    BrowserWindow window;
    window.show();

    return app.exec();
}