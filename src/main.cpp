#include <QApplication>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // 애플리케이션 정보 설정
    app.setApplicationName("Vision Pre Comparison Tool");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("FITO");

    // 스타일 설정 (선택사항)
    app.setStyle("Fusion");

    // 메인 윈도우 생성 및 표시
    MainWindow mainWindow;
    mainWindow.setWindowTitle("Vision Pre Comparison Tool - FITO 2026");
    mainWindow.resize(1600, 900);
    mainWindow.show();

    return app.exec();
}
