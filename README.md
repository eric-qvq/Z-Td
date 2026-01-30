# 📝 Qt To-Do List

![Qt](https://img.shields.io/badge/Qt-6.0%2B-green.svg) ![C++](https://img.shields.io/badge/C%2B%2B-17-blue.svg) ![License](https://img.shields.io/badge/License-MIT-yellow.svg) ![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux-lightgrey)

一个基于 **C++** 和 **Qt 6** 框架开发的现代化桌面待办事项管理应用。
不仅具备基础的增删改查功能，还集成了系统托盘、黑夜模式、拖拽排序等高级特性，旨在提供轻量、高效的任务管理体验。

## ✨ 功能特性 (Features)

### 核心功能
- ✅ **任务管理**：支持添加、删除任务，双击任务即可**编辑内容**。
- 📅 **截止日期**：支持为任务设置截止日期，直观展示时间安排。
- 💾 **数据持久化**：使用 **JSON** 格式本地存储数据，程序关闭后数据不丢失。
- 🖱️ **拖拽排序**：支持通过鼠标拖拽 (Drag & Drop) 自由调整任务优先级。

### 用户体验优化
- 🌗 **深色/浅色模式**：一键切换主题，支持持久化保存用户的主题偏好。
- 🔍 **实时搜索**：顶部搜索栏支持实时过滤任务，快速定位。
- ⏰ **实时时钟**：界面集成动态跳动的系统时间显示。
- ☑️ **一键清理**：快速移除所有已完成的任务。

### 系统级集成
- 📥 **系统托盘**：程序可最小化至系统托盘，支持后台运行。
- ⚙️ **记忆功能**：自动记忆窗口大小、位置以及“关闭时是否最小化”的用户设置。
- 🖱️ **右键菜单**：提供便捷的右键上下文菜单（编辑/删除）。

## 🛠️ 技术栈 (Tech Stack)

- **编程语言**: C++ 17
- **GUI 框架**: Qt 6 (Widgets 模块)
- **构建工具**: CMake
- **数据格式**: JSON (`QJsonDocument`, `QJsonObject`)
- **配置存储**: `QSettings` (注册表/INI)
- **开发环境**: VS Code + MinGW / MSVC

## 🚀 快速开始 (Build & Run)

### 前置要求
确保你的电脑上已安装：
1.  **C++ 编译器** (MinGW 或 MSVC)
2.  **CMake** (3.16 或更高版本)
3.  **Qt 6 SDK**

### 构建步骤

```bash
# 1. 克隆项目
git clone [https://github.com/你的用户名/Qt-ToDo-List.git](https://github.com/你的用户名/Qt-ToDo-List.git)
cd Qt-ToDo-List

# 2. 创建构建目录
mkdir build
cd build

# 3. 生成构建文件 (确保 CMake 能找到 Qt)
cmake ..

# 4. 编译
cmake --build .

# 5. 运行
./Z-Td.exe