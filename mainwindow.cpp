#include "mainwindow.h"
#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QMessageBox>
#include <QVBoxLayout>

// 改用 json 后缀
const QString DATA_FILENAME = "todo_data.json";

MainWindow::MainWindow(QWidget *parent) : QWidget(parent) {
    this->setWindowTitle("Z-Td List");
    this->resize(400, 600);

    setupUi();
    setupTrayIcon(); // 设置系统托盘图标

    QTimer *timer = new QTimer(this);

    connect(timer, &QTimer::timeout, this, [=]() {
        // 获取当前系统时间
        QDateTime current = QDateTime::currentDateTime();
        // 格式化为：年-月-日 时:分:秒 星期几
        QString timeStr = current.toString("yyyy-MM-dd HH:mm:ss dddd");
        timeLabel->setText(timeStr);
    });

    timer->start(1000); // 启动定时器，间隔 1000ms (1秒)

    QDateTime current = QDateTime::currentDateTime();
    timeLabel->setText(current.toString("yyyy-MM-dd HH:mm:ss dddd"));

    taskList->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(themeButton, &QPushButton::clicked, this, &MainWindow::toggleTheme);
    connect(taskList, &QListWidget::customContextMenuRequested, this, &MainWindow::showContextMenu);
    connect(taskList, &QListWidget::itemDoubleClicked, this, &MainWindow::editTask);
    connect(addButton, &QPushButton::clicked, this, &MainWindow::addTask);
    connect(inputBox, &QLineEdit::returnPressed, this, &MainWindow::addTask);
    connect(taskList, &QListWidget::itemChanged, this, &MainWindow::saveTasks);
    connect(clearButton, &QPushButton::clicked, [=]() {
        for (int i = taskList->count() - 1; i >= 0; --i) {
            QListWidgetItem *item = taskList->item(i);
            if (item->checkState() == Qt::Checked) {
                delete taskList->takeItem(i); // 移除并删除
            }
        }
        saveTasks(); // 保存更改
    });
    connect(searchBox, &QLineEdit::textChanged, [=](const QString &text) {
        // 遍历每一行任务
        for (int i = 0; i < taskList->count(); ++i) {
            QListWidgetItem *item = taskList->item(i);
            // 如果包含关键词 (不区分大小写)，就显示；否则隐藏
            bool isMatch = item->text().contains(text, Qt::CaseInsensitive);
            item->setHidden(!isMatch);
        }
    });
    connect(taskList->model(), &QAbstractItemModel::rowsMoved, this, &MainWindow::saveTasks);

    loadTasks();
    loadSettings(); // <--- 新增：加载软件设置 (复选框状态)
}

MainWindow::~MainWindow() {
}

void MainWindow::setupUi() {
    QLabel *titleLabel = new QLabel("今日待办事项", this);
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; margin-bottom: 10px;");
    titleLabel->setAlignment(Qt::AlignCenter);

    inputBox = new QLineEdit(this);
    inputBox->setPlaceholderText("请输入任务...");
    inputBox->setStyleSheet("padding: 5px;");

    addButton = new QPushButton("添加", this);
    addButton->setStyleSheet("background-color: #007ACC; color: white; padding: 5px; font-weight: bold;");

    // --- 新增：清理按钮 ---
    clearButton = new QPushButton("清理已完成", this);
    clearButton->setStyleSheet("background-color: #007ACC; color: white; padding: 5px;");

    minimizeCheckBox = new QCheckBox("关闭时最小化到托盘", this);
    minimizeCheckBox->setChecked(true); // 默认勾选（默认行为是最小化）
    minimizeCheckBox->setStyleSheet("font-size: 12px; color: #666; margin-bottom: 5px;");

    taskList = new QListWidget(this);
    taskList->setStyleSheet("font-size: 14px; border: 1px solid #ccc; border-radius: 4px;");

    searchBox = new QLineEdit(this);
    searchBox->setPlaceholderText("🔍 搜索任务...");
    searchBox->setStyleSheet("padding: 5px; border-radius: 15px; border: 1px solid #ccc;");

    themeButton = new QPushButton("🌙 切换主题", this);
    themeButton->setStyleSheet("border: none; background: transparent; font-weight: bold; color: #555;");
    themeButton->setCursor(Qt::PointingHandCursor);

    dateEdit = new QDateEdit(QDate::currentDate(), this);
    dateEdit->setCalendarPopup(true); // 开启日历弹窗
    dateEdit->setDisplayFormat("yyyy-MM-dd");
    dateEdit->setStyleSheet("padding: 5px; border-radius: 5px; border: 1px solid #ccc;");

    timeLabel = new QLabel(this);
    timeLabel->setStyleSheet("font-size: 16px; font-weight: bold; font-family: Consolas, Monospace;");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QHBoxLayout *inputLayout = new QHBoxLayout();

    QHBoxLayout *topLayout = new QHBoxLayout();
    topLayout->addWidget(timeLabel);   // 1. 左边放时间
    topLayout->addStretch();           // 2. 中间放弹簧 (把两边顶开)
    topLayout->addWidget(themeButton); // 3. 右边放主题按钮

    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(searchBox);

    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(searchBox);

    mainLayout->addWidget(searchBox);
    mainLayout->addWidget(titleLabel);

    taskList->setSelectionMode(QAbstractItemView::SingleSelection); // 单选模式
    taskList->setDragEnabled(true);                                 // 允许拖
    taskList->setAcceptDrops(true);                                 // 允许放
    taskList->setDropIndicatorShown(true);                          // 显示插入位置的横线
    taskList->setDragDropMode(QAbstractItemView::InternalMove);     // 关键：只能在内部移动，不能拖出去

    inputLayout->addWidget(inputBox);
    inputLayout->addWidget(dateEdit);
    inputLayout->addWidget(addButton);
    inputLayout->addWidget(clearButton);

    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(minimizeCheckBox);
    mainLayout->addLayout(inputLayout);
    mainLayout->addWidget(taskList);
}

void MainWindow::addTask() {
    QString text = inputBox->text().trimmed();
    QString date = dateEdit->text(); // 获取日期

    if (text.isEmpty())
        return;

    QString displayText = QString("[%1] %2").arg(date).arg(text);

    QListWidgetItem *item = new QListWidgetItem(displayText);
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
    item->setCheckState(Qt::Unchecked);

    item->setData(Qt::UserRole, text);     // 存纯标题
    item->setData(Qt::UserRole + 1, date); // 存日期

    taskList->addItem(item);

    inputBox->clear();
    // dateEdit->setDate(QDate::currentDate()); // 可选：重置日期
    saveTasks();
}

void MainWindow::deleteTask(QListWidgetItem *item) {
    int ret = QMessageBox::question(this, "确认", "确定要删除这条任务吗？", QMessageBox::Yes | QMessageBox::No);
    if (ret == QMessageBox::Yes) {
        delete item;
        saveTasks();
    }
}

// --- 核心升级：保存为 JSON ---
void MainWindow::saveTasks() {
    QString path = QCoreApplication::applicationDirPath() + "/" + DATA_FILENAME;
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly))
        return;
    QJsonArray jsonArray;

    for (int i = 0; i < taskList->count(); ++i) {
        QListWidgetItem *item = taskList->item(i);
        QJsonObject taskObj;

        // 提取已有信息
        taskObj["title"] = item->data(Qt::UserRole).toString();    // 注意：这里我们改用 UserRole 存纯文本
        taskObj["date"] = item->data(Qt::UserRole + 1).toString(); // 用 UserRole+1 存日期
        taskObj["done"] = (item->checkState() == Qt::Checked);

        jsonArray.append(taskObj);
    }
    QJsonDocument doc(jsonArray);
    file.write(doc.toJson());
    file.close();
}

// --- 核心升级：从 JSON 加载 ---
void MainWindow::loadTasks() {
    QString path = QCoreApplication::applicationDirPath() + "/" + DATA_FILENAME;
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return;

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonArray jsonArray = doc.array();

    for (const QJsonValue &value : jsonArray) {
        QJsonObject taskObj = value.toObject();

        QString title = taskObj["title"].toString();
        QString date = taskObj["date"].toString();
        bool isDone = taskObj["done"].toBool();

        // 如果是旧数据没有 title 字段（兼容性处理）
        if (title.isEmpty())
            title = "旧任务";

        QString displayText = QString("[%1] %2").arg(date).arg(title);
        QListWidgetItem *item = new QListWidgetItem(displayText);

        item->setFlags(item->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
        item->setCheckState(isDone ? Qt::Checked : Qt::Unchecked);

        // 恢复隐藏数据
        item->setData(Qt::UserRole, title);
        item->setData(Qt::UserRole + 1, date);

        taskList->addItem(item);
    }
    file.close();
}

// --- 新增：初始化托盘图标和菜单 ---
void MainWindow::setupTrayIcon() {
    // 1. 创建托盘菜单 (右键点击图标时显示)
    trayMenu = new QMenu(this);

    // 添加 "显示主界面" 动作
    QAction *restoreAction = trayMenu->addAction("显示主界面");
    connect(restoreAction, &QAction::triggered, this, &MainWindow::showNormal);

    // 添加 "退出" 动作
    QAction *quitAction = trayMenu->addAction("退出");
    connect(quitAction, &QAction::triggered, [=]() {
        saveSettings(); // 保存复选框状态
        qApp->quit();   // 退出程序
    });

    // 2. 创建托盘图标
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(QIcon("logo.ico")); // 一定要用你的那个图标文件
    trayIcon->setContextMenu(trayMenu);   // 设置右键菜单
    trayIcon->setToolTip("我的今日待办"); // 鼠标悬停提示

    // 3. 点击托盘图标的操作 (左键点击显示/隐藏)
    connect(trayIcon, &QSystemTrayIcon::activated, this, [=](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::Trigger) { // 单击
            if (this->isVisible()) {
                this->hide();
            } else {
                this->showNormal();
                this->activateWindow(); // 放到最顶层
            }
        }
    });

    // 4. 显示托盘图标
    trayIcon->show();
}

void MainWindow::closeEvent(QCloseEvent *event) {
    // 先判断是否要最小化
    if (minimizeCheckBox->isChecked()) {
        this->hide();
        event->ignore();
    } else {
        // 如果要退出了，赶紧保存设置！
        saveSettings(); // <--- 新增
        event->accept();
    }
}

// --- 新增：显示右键菜单 ---
void MainWindow::showContextMenu(const QPoint &pos) {
    // 1. 获取鼠标点击位置的任务项
    QListWidgetItem *item = taskList->itemAt(pos);
    if (!item)
        return; // 如果点在空白处，不显示菜单

    // 2. 创建菜单
    QMenu menu(this);
    QAction *editAction = menu.addAction("✏️ 编辑");
    QAction *deleteAction = menu.addAction("🗑️ 删除");

    // 3. 连接菜单动作
    // 使用 Lambda 表达式来处理点击
    connect(editAction, &QAction::triggered, [=]() {
        editTask(item); // 调用编辑函数
    });

    connect(deleteAction, &QAction::triggered, [=]() {
        deleteTask(item); // 调用删除函数
    });

    // 4. 在鼠标位置弹出菜单
    menu.exec(taskList->mapToGlobal(pos));
}

// --- 新增：编辑任务逻辑 ---
void MainWindow::editTask(QListWidgetItem *item) {
    bool ok;
    // 弹出输入框，默认填入旧文字
    QString oldText = item->text();
    QString newText = QInputDialog::getText(this, "修改任务", "请输入新的内容:", QLineEdit::Normal, oldText, &ok);

    // 如果用户点了确定(ok) 且 内容不为空
    if (ok && !newText.trimmed().isEmpty()) {
        item->setText(newText.trimmed()); // 更新界面
        saveTasks();                      // 保存到文件
    }
}

// --- 新增：保存设置 ---
void MainWindow::saveSettings() {
    // 创建 QSettings 对象
    // 参数1：公司/组织名 (随便填)
    // 参数2：软件名 (随便填)
    QSettings settings("MySoft", "ToDoList");

    // 保存复选框的状态
    settings.setValue("minimizeToTray", minimizeCheckBox->isChecked());

    // 【可选】顺便保存一下窗口大小和位置，体验更好
    settings.setValue("geometry", saveGeometry());
}

// --- 新增：读取设置 ---
void MainWindow::loadSettings() {
    QSettings settings("MySoft", "ToDoList");
    bool isMinimize = settings.value("minimizeToTray", true).toBool();
    minimizeCheckBox->setChecked(isMinimize);
    isDarkMode = settings.value("darkMode", false).toBool();
    updateThemeStyle();
    if (settings.contains("geometry")) {
        restoreGeometry(settings.value("geometry").toByteArray());
    }
}
void MainWindow::toggleTheme() {
    isDarkMode = !isDarkMode; // 取反状态
    updateThemeStyle();       // 刷新界面

    // 更新按钮文字
    themeButton->setText(isDarkMode ? "☀️ 亮色模式" : "🌙 暗色模式");

    // 保存设置 (记得在 loadSettings 里也要读取这个值哦)
    QSettings settings("MySoft", "ToDoList");
    settings.setValue("darkMode", isDarkMode);
}

void MainWindow::updateThemeStyle() {
    QString style;
    if (isDarkMode) {
        // --- 黑夜模式样式 ---
        style = R"(
            QWidget { background-color: #2b2b2b; color: #e0e0e0; font-family: "Microsoft YaHei"; }
            QLineEdit { background-color: #3c3f41; border: 1px solid #555; border-radius: 8px; padding: 8px; color: white; }
            QListWidget { background-color: #3c3f41; border: 1px solid #555; border-radius: 10px; padding: 10px; }
            QListWidget::item { border-bottom: 1px solid #555; }
            QListWidget::item:selected { background-color: #4b6eaf; }
            QPushButton { background-color: #365880; color: white; border-radius: 6px; padding: 6px; }
            QPushButton:hover { background-color: #4b6eaf; }
        )";
    } else {
        // --- 亮色模式样式 (原来的) ---
        style = R"(
            QWidget { background-color: #f5f7fa; color: #333; font-family: "Microsoft YaHei"; }
            QLineEdit { background-color: white; border: 1px solid #ccc; border-radius: 8px; padding: 8px; color: #333; }
            QListWidget { background-color: white; border: 1px solid #ccc; border-radius: 10px; padding: 10px; }
            QListWidget::item { border-bottom: 1px solid #eee; }
            QListWidget::item:selected { background-color: #e6f2ff; color: #007bff; }
            QPushButton { background-color: #007ACC; color: white; border-radius: 6px; padding: 6px; }
            QPushButton:hover { background-color: #0056b3; }
        )";
    }
    // 应用到全局
    qApp->setStyleSheet(style);
}