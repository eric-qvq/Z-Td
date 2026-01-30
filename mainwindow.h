#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QAction>
#include <QCheckBox>
#include <QCloseEvent>
#include <QDateEdit>
#include <QDateTime>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QPushButton>
#include <QSettings>
#include <QSystemTrayIcon>
#include <QTimer>
#include <QWidget>
class MainWindow : public QWidget {
    Q_OBJECT

  public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

  protected:
    void closeEvent(QCloseEvent *event) override;

  private:
    QLabel *timeLabel;
    QDateEdit *dateEdit;
    QListWidget *taskList;
    QLineEdit *inputBox;
    QPushButton *addButton;
    QPushButton *clearButton;
    QCheckBox *minimizeCheckBox;
    QSystemTrayIcon *trayIcon;
    QMenu *trayMenu;
    QLineEdit *searchBox;
    QPushButton *themeButton; // 切换主题的按钮
    bool isDarkMode = false;  // 记录当前是不是黑夜模式

    void toggleTheme();      // 切换主题的函数
    void updateThemeStyle(); // 刷新样式的函数

    void showContextMenu(const QPoint &pos); // 显示右键菜单
    void editTask(QListWidgetItem *item);    // 编辑任务
    void setupUi();
    void setupTrayIcon(); // 专门用来初始化托盘的函数
    void loadTasks();
    void saveTasks();
    void addTask();
    void deleteTask(QListWidgetItem *item);
    void loadSettings(); // 启动时读取
    void saveSettings(); // 关闭时保存
};

#endif // MAINWINDOW_H