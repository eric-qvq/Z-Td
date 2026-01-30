#ifndef __TODOLIST__H__
#define __TODOLIST__H__

#include <string>
#include <vector>

// 声明 Task 结构体
struct Task {
    std::string description;
    bool isCompleted;
};

// 声明 TodoApp 类
class TodoApp {
  private:
    std::vector<Task> tasks;
    const std::string filename = "todo_list.txt";

    void saveToFile();
    void loadFromFile();

  public:
    TodoApp(); // 构造函数
    void addTask();
    void showTasks();
    void toggleTask();
    void deleteTask();
};

#endif //!__TODOLIST__H__