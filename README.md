# 📝 Qt Modern To-Do List

![Qt](https://img.shields.io/badge/Qt-6.0%2B-green.svg) ![C++](https://img.shields.io/badge/C%2B%2B-17-blue.svg) ![Network](https://img.shields.io/badge/Qt-Network-orange.svg) ![API](https://img.shields.io/badge/API-Open--Meteo-yellow.svg)

一个基于 **C++** 和 **Qt 6** 开发的现代化桌面待办事项管理仪表盘。
本项目不仅仅是一个列表工具，更集成了**实时天气获取**、**系统托盘驻留**、**黑夜模式切换**以及**拖拽排序**等高级功能，旨在打造一个全能的个人效率中心。

## ✨ 核心特性 (Features)

### 🖥️ 现代化仪表盘 (Dashboard UI)
- **全新布局**：采用“控制台”式设计，将时间、天气、操作按钮与输入区合理分区。
- **沉浸式输入**：超大尺寸的独立输入栏，提供极佳的输入专注度。
- **动态交互**：按钮和输入框经过 CSS 深度美化，支持悬停 (Hover) 与按压 (Pressed) 动态效果。

### ☁️ 联网功能 (Network & API)
- **实时天气**：集成 `Qt Network` 模块，调用 **Open-Meteo API** 实时获取当地（默认台北）的气温与天气状况。
- **异步加载**：采用非阻塞式网络请求，确保界面流畅不卡顿。

### 🛠️ 任务管理 (Task Management)
- **增删改查**：支持快速添加、双击编辑、一键清理已完成任务。
- **日期规划**：内置日历控件 (`QDateEdit`)，为每个任务设定截止日期。
- **拖拽排序**：支持通过鼠标拖拽 (Drag & Drop) 自由调整任务优先级。
- **实时搜索**：顶部搜索栏支持关键词实时过滤。

### ⚙️ 系统集成与体验
- **黑夜模式**：内置 Light/Dark 两套主题，一键切换并自动记忆。
- **系统托盘**：支持最小化到托盘，程序可常驻后台运行。
- **数据持久化**：使用 JSON 格式本地存储任务数据与用户设置。

## 🛠️ 技术栈 (Tech Stack)

- **语言**: C++ 17
- **框架**: Qt 6 (Widgets, Network)
- **构建工具**: CMake
- **数据存储**: JSON (`QJsonDocument`)
- **网络通信**: `QNetworkAccessManager` (REST API)
- **第三方服务**: [Open-Meteo API](https://open-meteo.com/) (无需 Key，免费开源)

## 🚀 快速开始 (Build & Run)

### 环境要求
1.  **C++ 编译器** (MinGW / MSVC)
2.  **CMake** (3.16+)
3.  **Qt 6 SDK** (需包含 `Qt Network` 模块)

### 构建步骤

```bash
# 1. 克隆项目
git clone https://github.com/eric-qvq/Z-Td.git
cd Z-Td

# 2. 创建构建目录
mkdir build
cd build

# 3. 生成构建文件 (确保 CMake 能找到 Qt)
cmake ..

# 4. 编译
cmake --build .

# 5. 运行
./Z-Td.exe