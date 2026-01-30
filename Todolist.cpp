// todo.cpp
#include "Todolist.h" // 引用刚才定义的头文件
#include <fstream>
#include <iostream>
#include <limits>

using namespace std;

// 构造函数实现
TodoApp::TodoApp() {
    loadFromFile();
}

// 具体功能的实现...
void TodoApp::saveToFile() {
    ofstream file(filename);
    if (file.is_open()) {
        for (const auto &task : tasks) {
            file << task.isCompleted << "|" << task.description << endl;
        }
        file.close();
    }
}

void TodoApp::loadFromFile() {
    ifstream file(filename);
    if (file.is_open()) {
        tasks.clear();
        string line;
        while (getline(file, line)) {
            size_t delimiterPos = line.find('|');
            if (delimiterPos != string::npos) {
                bool completed = (line.substr(0, delimiterPos) == "1");
                string desc = line.substr(delimiterPos + 1);
                tasks.push_back({desc, completed});
            }
        }
        file.close();
    }
}

void TodoApp::addTask() {
    cout << "请输入任务描述: ";
    string desc;
    cin.ignore();
    getline(cin, desc);
    if (!desc.empty()) {
        tasks.push_back({desc, false});
        saveToFile();
        cout << ">> 任务已添加！" << endl;
    }
}

void TodoApp::showTasks() {
    cout << "\n================ 今日待办 ================" << endl;
    if (tasks.empty()) {
        cout << "  (目前没有任务)" << endl;
    } else {
        for (size_t i = 0; i < tasks.size(); ++i) {
            cout << " " << i + 1 << ". [" << (tasks[i].isCompleted ? "X" : " ") << "] " << tasks[i].description << endl;
        }
    }
    cout << "==========================================" << endl;
}

void TodoApp::toggleTask() {
    showTasks();
    if (tasks.empty())
        return;
    cout << "输入序号: ";
    int index;
    if (cin >> index && index > 0 && index <= tasks.size()) {
        tasks[index - 1].isCompleted = !tasks[index - 1].isCompleted;
        saveToFile();
    } else {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

void TodoApp::deleteTask() {
    showTasks();
    if (tasks.empty())
        return;
    cout << "输入删除序号: ";
    int index;
    if (cin >> index && index > 0 && index <= tasks.size()) {
        tasks.erase(tasks.begin() + index - 1);
        saveToFile();
    } else {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}