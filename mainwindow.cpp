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

    // 1. 初始化网络管理者
    netManager = new QNetworkAccessManager(this);

    // 2. 连接“请求完成”的信号
    // 当网络请求回来时，自动执行这个 Lambda 函数
    connect(netManager, &QNetworkAccessManager::finished, this, [=](QNetworkReply *reply) {
        if (reply->error() == QNetworkReply::NoError) {
            // A. 读取数据
            QByteArray response = reply->readAll();

            // B. 解析 JSON
            QJsonDocument doc = QJsonDocument::fromJson(response);
            QJsonObject obj = doc.object();
            QJsonObject current = obj["current_weather"].toObject();

            // C. 提取温度和天气代码
            double temp = current["temperature"].toDouble();
            int weatherCode = current["weathercode"].toInt();

            // D. 更新 UI
            QString emoji = getWeatherEmoji(weatherCode);
            weatherLabel->setText(QString("%1 %2°C").arg(emoji).arg(temp));
        } else {
            weatherLabel->setText("❌ 网络错误");
        }
        reply->deleteLater(); // 释放内存
    });

    setupUi();
    setupTrayIcon(); // 设置系统托盘图标
    fetchWeather();

    // 4. (可选) 设置定时器，每 1 小时自动刷新一次
    QTimer *weatherTimer = new QTimer(this);
    connect(weatherTimer, &QTimer::timeout, this, &MainWindow::fetchWeather);
    weatherTimer->start(3600 * 1000); // 3600秒 = 1小时

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

/* void MainWindow::setupUi() {
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
} */

/* void MainWindow::setupUi() {
    // ================= 1. 初始化控件 & 设置样式 =================

    // --- 1. 时间标签 (保持在最顶层左边) ---
    timeLabel = new QLabel(this);
    timeLabel->setStyleSheet("font-size: 16px; font-weight: bold; font-family: Consolas, Monospace; color: #555;");

    // --- 2. 主题切换按钮 (保持在最顶层右边) ---
    themeButton = new QPushButton("🌙 切换主题", this);
    themeButton->setStyleSheet("border: none; background: transparent; font-weight: bold; color: #555;");
    themeButton->setCursor(Qt::PointingHandCursor);

    // --- 3. 搜索框 ---
    searchBox = new QLineEdit(this);
    searchBox->setPlaceholderText("🔍 搜索任务...");
    searchBox->setStyleSheet("padding: 6px; border-radius: 15px; border: 1px solid #ddd;");

    // --- 4. 标题 ---
    QLabel *titleLabel = new QLabel("今日待办事项", this);
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; margin: 10px 0; color: #333;");
    titleLabel->setAlignment(Qt::AlignCenter);

    // --- 5. 日期选择器 (放在标题下面) ---
    dateEdit = new QDateEdit(QDate::currentDate(), this);
    dateEdit->setCalendarPopup(true);
    dateEdit->setDisplayFormat("yyyy-MM-dd");
    // 设置一点高度，不要太矮
    dateEdit->setMinimumHeight(35);
    dateEdit->setStyleSheet("padding: 0 10px; border: 1px solid #ccc; border-radius: 6px; color: #333;");

    // --- 6. 功能按钮 (添加 & 清理) ---
    addButton = new QPushButton("添加任务", this);
    addButton->setStyleSheet(
        "background-color: #007ACC; color: white; border-radius: 6px; padding: 6px 20px; font-weight: bold;");
    addButton->setMinimumHeight(35);

    clearButton = new QPushButton("🧹 清理已完成", this);
    clearButton->setStyleSheet(
        "background-color: #E0E0E0; color: #333; border-radius: 6px; padding: 6px 10px; border: 1px solid #ccc;");
    clearButton->setMinimumHeight(35);

    // --- 7. 输入框 (重点：单独一栏，放大) ---
    inputBox = new QLineEdit(this);
    inputBox->setPlaceholderText("在此输入新的待办事项内容...");
    inputBox->setMinimumHeight(50); // 【关键】设置高度为 50px，显眼！
    inputBox->setStyleSheet(
        "font-size: 18px; padding: 0 15px; border: 2px solid #007ACC; border-radius: 8px; background-color: white;");

    // --- 8. 最小化选项 ---
    minimizeCheckBox = new QCheckBox("关闭时最小化到托盘", this);
    minimizeCheckBox->setChecked(true);
    minimizeCheckBox->setStyleSheet("font-size: 12px; color: #666; margin-bottom: 5px;");

    // --- 9. 任务列表 ---
    taskList = new QListWidget(this);
    taskList->setStyleSheet(
        "font-size: 15px; border: 1px solid #eee; border-radius: 10px; padding: 5px; outline: none;");
    taskList->setSelectionMode(QAbstractItemView::SingleSelection);
    taskList->setDragEnabled(true);
    taskList->setAcceptDrops(true);
    taskList->setDropIndicatorShown(true);
    taskList->setDragDropMode(QAbstractItemView::InternalMove);

    // ================= 2. 组装布局 (Layout) =================

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(12); // 控件垂直间距
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // --- 第 1 行：时间 + 主题 (TopBar) ---
    QHBoxLayout *topLayout = new QHBoxLayout();
    topLayout->addWidget(timeLabel);   // 左：时间
    topLayout->addStretch();           // 中：弹簧
    topLayout->addWidget(themeButton); // 右：主题
    mainLayout->addLayout(topLayout);

    // --- 第 2 行：搜索框 ---
    mainLayout->addWidget(searchBox);

    // --- 第 3 行：大标题 ---
    mainLayout->addWidget(titleLabel);

    // --- 第 4 行：操作栏 (日历 + 添加 + 清理) ---
    QHBoxLayout *controlLayout = new QHBoxLayout();
    controlLayout->addWidget(dateEdit);    // 日历
    controlLayout->addWidget(addButton);   // 添加
    controlLayout->addStretch();           // 弹簧 (让清理按钮靠右，或者去掉这行让它们紧挨着)
    controlLayout->addWidget(clearButton); // 清理
    mainLayout->addLayout(controlLayout);

    // --- 第 5 行：输入框 (大、独占) ---
    mainLayout->addWidget(inputBox);

    // --- 第 6 行：选项 ---
    mainLayout->addWidget(minimizeCheckBox);

    // --- 第 7 行：列表 ---
    mainLayout->addWidget(taskList);
} */

void MainWindow::setupUi() {
    // ================= 1. 初始化控件 & 设置样式 =================

    // --- 1. 时间标签 ---
    timeLabel = new QLabel(this);
    timeLabel->setStyleSheet("font-size: 16px; font-weight: bold; font-family: Consolas, Monospace; color: #555;");

    weatherLabel = new QLabel(this);
    weatherLabel->setText("🌤️ 22°C 晴"); // 这里先写死，作为演示
    weatherLabel->setStyleSheet("font-size: 16px;"
                                "font-weight: bold;"
                                "color: #555;"
                                "margin-left: 15px;" // 关键：给左边加点间距，别和时间挤在一起
                                "font-family: 'Microsoft YaHei';");

    // --- 2. 主题切换按钮 ---
    themeButton = new QPushButton("🌙 切换主题", this);
    themeButton->setStyleSheet("QPushButton { border: none; background: transparent; font-weight: bold; color: #666; }"
                               "QPushButton:hover { color: #007ACC; }" // 悬停变蓝
    );
    themeButton->setCursor(Qt::PointingHandCursor);

    // --- 3. 搜索框 ---
    searchBox = new QLineEdit(this);
    searchBox->setPlaceholderText("🔍 搜索任务...");
    searchBox->setStyleSheet("padding: 6px; border-radius: 15px; border: 1px solid #ddd; background: white;");

    // --- 4. 标题 ---
    QLabel *titleLabel = new QLabel("今日待办事项", this);
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; margin: 15px 0; color: #333;");
    titleLabel->setAlignment(Qt::AlignCenter);

    // --- 5. 日期选择器 (大整容) ---
    dateEdit = new QDateEdit(QDate::currentDate(), this);
    dateEdit->setCalendarPopup(true);
    dateEdit->setDisplayFormat("yyyy-MM-dd");
    dateEdit->setMinimumHeight(38); // 稍微高一点
    // 【CSS 魔法】
    dateEdit->setStyleSheet("QDateEdit {"
                            "   padding-left: 10px;"
                            "   border: 1px solid #ccc;"
                            "   border-radius: 6px;"
                            "   color: #333;"
                            "   background: white;"
                            "}"
                            // 右边的下拉按钮样式
                            "QDateEdit::drop-down {"
                            "   subcontrol-origin: padding;"
                            "   subcontrol-position: top right;"
                            "   width: 30px;"
                            "   border-left: 1px solid #ccc;" /* 左边加一条线分割 */
                            "   border-top-right-radius: 6px;"
                            "   border-bottom-right-radius: 6px;"
                            "   background-color: #f5f5f5;" /* 按钮背景稍微灰一点 */
                            "}"
                            "QDateEdit::drop-down:hover {"
                            "   background-color: #e0e0e0;" /* 悬停变深 */
                            "}"
                            // 下箭头的样式 (虽然没放图片，但利用 CSS 让它看起来干净)
                            "QDateEdit::down-arrow {"
                            "   width: 10px; height: 10px;"
                            // 这里如果有图片可以是 image: url(:/icon.png);
                            // 没有图片时，Qt 默认会画一个小三角，我们通过上面的背景色让它显眼即可
                            "}");

    // --- 6. 功能按钮 (加 Emoji + 动态效果) ---
    addButton = new QPushButton("➕ 添加任务", this); // 加上 Emoji
    addButton->setMinimumHeight(38);
    addButton->setCursor(Qt::PointingHandCursor);
    addButton->setStyleSheet("QPushButton {"
                             "   background-color: #007ACC;"
                             "   color: white;"
                             "   border-radius: 6px;"
                             "   padding: 0 20px;"
                             "   font-weight: bold;"
                             "   font-size: 14px;"
                             "}"
                             "QPushButton:hover {"
                             "   background-color: #0062a3;" /* 悬停变深蓝 */
                             "}"
                             "QPushButton:pressed {"
                             "   background-color: #004472;" /* 按下更深 */
                             "   padding-top: 2px;"          /* 只有按下时文字下移，产生按压感 */
                             "}");

    clearButton = new QPushButton("🗑️ 清理完成", this); // 加上 Emoji
    clearButton->setMinimumHeight(38);
    clearButton->setCursor(Qt::PointingHandCursor);
    clearButton->setStyleSheet("QPushButton {"
                               "   background-color: #f0f0f0;"
                               "   color: #333;"
                               "   border: 1px solid #ccc;"
                               "   border-radius: 6px;"
                               "   padding: 0 15px;"
                               "   font-weight: bold;"
                               "}"
                               "QPushButton:hover {"
                               "   background-color: #e6e6e6;" /* 悬停变灰 */
                               "   border-color: #bbb;"
                               "}"
                               "QPushButton:pressed {"
                               "   background-color: #dcdcdc;"
                               "   padding-top: 2px;"
                               "}");

    // --- 7. 输入框 (大、白、净) ---
    inputBox = new QLineEdit(this);
    inputBox->setPlaceholderText("✍️ 在此输入新的待办事项内容..."); // 加个笔的 Emoji
    inputBox->setMinimumHeight(50);
    inputBox->setStyleSheet("QLineEdit {"
                            "   font-size: 18px;"
                            "   padding: 0 15px;"
                            "   border: 2px solid #e0e6ed;" /* 默认边框浅蓝灰 */
                            "   border-radius: 8px;"
                            "   background-color: white;"
                            "   color: #333;"
                            "}"
                            "QLineEdit:focus {"
                            "   border: 2px solid #007ACC;" /* 聚焦时变亮蓝 */
                            "}");

    // --- 8. 最小化选项 ---
    minimizeCheckBox = new QCheckBox("关闭时最小化到托盘", this);
    minimizeCheckBox->setChecked(true);
    minimizeCheckBox->setStyleSheet("QCheckBox { font-size: 12px; color: #666; margin-bottom: 5px; }"
                                    "QCheckBox::indicator { width: 16px; height: 16px; }" /* 放大一点勾选框 */
    );

    // --- 9. 任务列表 ---
    taskList = new QListWidget(this);
    taskList->setStyleSheet("QListWidget {"
                            "   font-size: 15px;"
                            "   border: 1px solid #eee;"
                            "   border-radius: 10px;"
                            "   padding: 5px;"
                            "   background-color: white;"
                            "   outline: none;"
                            "}"
                            "QListWidget::item {"
                            "   padding: 8px;"
                            "   border-bottom: 1px solid #f9f9f9;"
                            "}"
                            "QListWidget::item:selected {"
                            "   background-color: #e6f2ff;"
                            "   color: #007ACC;"
                            "   border-radius: 4px;"
                            "}"
                            "QListWidget::item:hover {"
                            "   background-color: #f5f7fa;" /* 鼠标划过微微变色 */
                            "}");
    taskList->setSelectionMode(QAbstractItemView::SingleSelection);
    taskList->setDragEnabled(true);
    taskList->setAcceptDrops(true);
    taskList->setDropIndicatorShown(true);
    taskList->setDragDropMode(QAbstractItemView::InternalMove);

    // ================= 2. 组装布局 (Layout) =================

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15); // 稍微增加间距，呼吸感更强
    mainLayout->setContentsMargins(25, 25, 25, 25);

    // Top
    QHBoxLayout *topLayout = new QHBoxLayout();
    topLayout->addWidget(timeLabel);    // 1. 时间
    topLayout->addWidget(weatherLabel); // 2. 天气 (加在这里！)
    topLayout->addStretch();            // 3. 弹簧 (把后面顶到最右边)
    topLayout->addWidget(themeButton);  // 4. 主题按钮
    mainLayout->addLayout(topLayout);

    // Search
    mainLayout->addWidget(searchBox);

    // Title
    mainLayout->addWidget(titleLabel);

    // Control Bar (日历 + 添加 + 清理)
    QHBoxLayout *controlLayout = new QHBoxLayout();
    controlLayout->setSpacing(10); // 按钮之间的间距
    controlLayout->addWidget(dateEdit);
    controlLayout->addWidget(addButton);
    controlLayout->addStretch();
    controlLayout->addWidget(clearButton);
    mainLayout->addLayout(controlLayout);

    // Input
    mainLayout->addWidget(inputBox);

    // Option
    mainLayout->addWidget(minimizeCheckBox);

    // List
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
        timeLabel->setStyleSheet("color: #e0e0e0; font-size: 16px; font-weight: bold; font-family: Consolas;");
        weatherLabel->setStyleSheet(
            "color: #e0e0e0; font-size: 16px; font-weight: bold; margin-left: 15px; font-family: 'Microsoft YaHei';");
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
        timeLabel->setStyleSheet("color: #555; font-size: 16px; font-weight: bold; font-family: Consolas;");
        weatherLabel->setStyleSheet(
            "color: #555; font-size: 16px; font-weight: bold; margin-left: 15px; font-family: 'Microsoft YaHei';");
    }
    qApp->setStyleSheet(style);
}

void MainWindow::fetchWeather() {
    // Open-Meteo API URL
    // latitude=纬度, longitude=经度
    QString url = "https://api.open-meteo.com/v1/forecast?"
                  "latitude=23.02&longitude=113.23" // <--- 这里修改你的坐标
                  "&current_weather=true";

    // 发送 GET 请求
    netManager->get(QNetworkRequest(QUrl(url)));
}
QString MainWindow::getWeatherEmoji(int code) {
    // 根据 WMO Weather Codes 转换
    // 0: 晴天, 1-3: 多云, 45-48: 雾, 51-67: 雨, 71-77: 雪, 95-99: 雷雨
    if (code == 0)
        return "☀️";
    if (code >= 1 && code <= 3)
        return "⛅";
    if (code >= 45 && code <= 48)
        return "🌫️";
    if (code >= 51 && code <= 67)
        return "🌧️";
    if (code >= 71 && code <= 77)
        return "❄️";
    if (code >= 80 && code <= 82)
        return "🌦️";
    if (code >= 95 && code <= 99)
        return "⛈️";
    return "🤷"; // 未知
}