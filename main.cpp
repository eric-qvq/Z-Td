#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // 设置全局样式 (依然放在这里)
    app.setWindowIcon(QIcon("logo.ico")); // 别忘了你的图标
    app.setStyleSheet(R"(
        QWidget { background-color: #f5f7fa; font-family: "Microsoft YaHei"; color: #333; }
        QLineEdit { background-color: white; border: 2px solid #e0e6ed; border-radius: 8px; padding: 8px; }
        QLineEdit:focus { border-color: #007bff; }
        QPushButton { background-color: #007bff; color: white; border-radius: 8px; padding: 8px 16px; border: none; }
        QPushButton:hover { background-color: #0056b3; }
        QListWidget { background-color: white; border: 1px solid #e0e6ed; border-radius: 10px; padding: 10px; outline: none; }
        QListWidget::item { padding: 10px; border-bottom: 1px solid #f0f0f0; }
        QListWidget::item:selected { background-color: #e6f2ff; color: #007bff; }
    )");

    // 实例化主窗口对象
    MainWindow w;
    w.show();

    return app.exec();
}