#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QTextStream>
#include <QVBoxLayout>
#include <QWidget>

// 定义数据文件名 (保存在程序同级目录下)
const QString DATA_FILENAME = "todo_data.txt";

// --- 辅助函数：保存数据到文件 ---
void saveTasks(QListWidget *taskList) {
    // 获取可执行文件所在的目录路径
    QString path = QCoreApplication::applicationDirPath() + "/" + DATA_FILENAME;
    QFile file(path);

    // 以“只写”和“清空原内容”的模式打开
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        // 遍历列表，一行行写入
        for (int i = 0; i < taskList->count(); ++i) {
            out << taskList->item(i)->text() << "\n";
        }
        file.close();
    }
}

// --- 辅助函数：从文件加载数据 ---
void loadTasks(QListWidget *taskList) {
    QString path = QCoreApplication::applicationDirPath() + "/" + DATA_FILENAME;
    QFile file(path);

    // 以“只读”模式打开
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();
            if (!line.isEmpty()) {
                taskList->addItem(line); // 直接添加到列表
            }
        }
        file.close();
    }
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    app.setWindowIcon(QIcon("logo.ico")); // 设置应用程序图标

    // --- 插入这段美化代码 ---
    app.setStyleSheet(R"(
        /* 全局背景色：柔和的灰白色 */
        QWidget {
            background-color: #f5f7fa;
            font-family: "Microsoft YaHei", "Segoe UI";
            color: #333;
        }

        /* 输入框：白色背景，圆角，微妙的阴影 */
        QLineEdit {
            background-color: white;
            border: 2px solid #e0e6ed;
            border-radius: 8px;
            padding: 8px 12px;
            font-size: 14px;
            selection-background-color: #007bff;
        }
        QLineEdit:focus {
            border-color: #007bff; /* 选中变蓝 */
        }

        /* 按钮：扁平化设计，蓝色渐变 */
        QPushButton {
            background-color: #007bff;
            color: white;
            border-radius: 8px;
            padding: 8px 16px;
            font-weight: bold;
            border: none;
        }
        QPushButton:hover {
            background-color: #0056b3; /* 鼠标悬停变深 */
        }
        QPushButton:pressed {
            background-color: #004494;
        }

        /* 列表：卡片式设计，去掉难看的边框 */
        QListWidget {
            background-color: white;
            border: 1px solid #e0e6ed;
            border-radius: 10px;
            padding: 10px;
            outline: none; /* 去掉选中时的虚线框 */
        }
        QListWidget::item {
            padding: 10px;
            border-bottom: 1px solid #f0f0f0;
            color: #555;
        }
        QListWidget::item:selected {
            background-color: #e6f2ff; /* 选中项的浅蓝色背景 */
            color: #007bff;
            border-radius: 5px;
        }
        QListWidget::item:hover {
            background-color: #f9f9f9;
        }
    )");
    // --- 美化代码结束 ---
    // 1. 创建主窗口
    QWidget window;
    window.setWindowTitle("Z-Td List");
    window.resize(400, 600);

    // 2. 创建 UI 控件
    QLabel *titleLabel = new QLabel("今日待办事项", &window);
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; margin-bottom: 10px;");
    titleLabel->setAlignment(Qt::AlignCenter);

    QLineEdit *inputBox = new QLineEdit(&window);
    inputBox->setPlaceholderText("请输入任务...");
    inputBox->setStyleSheet("padding: 5px;");

    QPushButton *addButton = new QPushButton("添加", &window);
    addButton->setStyleSheet("background-color: #007ACC; color: white; padding: 5px; font-weight: bold;");

    QListWidget *taskList = new QListWidget(&window);
    taskList->setStyleSheet("font-size: 14px; border: 1px solid #ccc; border-radius: 4px;");

    // 3. 布局
    QVBoxLayout *mainLayout = new QVBoxLayout(&window);
    QHBoxLayout *inputLayout = new QHBoxLayout();

    inputLayout->addWidget(inputBox);
    inputLayout->addWidget(addButton);

    mainLayout->addWidget(titleLabel);
    mainLayout->addLayout(inputLayout);
    mainLayout->addWidget(taskList);

    // 4. 核心逻辑

    // 定义“添加任务”的逻辑
    auto addTask = [&]() {
        QString text = inputBox->text().trimmed();
        if (!text.isEmpty()) {
            taskList->addItem("☐ " + text); // 加个框框前缀
            inputBox->clear();
            inputBox->setFocus();

            // 【关键点】数据变动，立即保存
            saveTasks(taskList);
        }
    };

    // 信号连接
    QObject::connect(addButton, &QPushButton::clicked, addTask);
    QObject::connect(inputBox, &QLineEdit::returnPressed, addTask);

    // 双击删除逻辑
    QObject::connect(taskList, &QListWidget::itemDoubleClicked, [&](QListWidgetItem *item) {
        int ret =
            QMessageBox::question(&window, "确认", "任务已完成？要删除它吗？", QMessageBox::Yes | QMessageBox::No);
        if (ret == QMessageBox::Yes) {
            delete item; // 从界面删除

            // 【关键点】数据变动，立即保存
            saveTasks(taskList);
        }
    });

    // 5. 【关键步骤】程序启动时，先加载数据
    loadTasks(taskList);

    window.show();
    return app.exec();
}