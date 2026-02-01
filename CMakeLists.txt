cmake_minimum_required(VERSION 3.16)

# 项目名称
project(Z-Td LANGUAGES CXX)

# 设置 C++ 标准 (Qt6 需要 C++17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# -----------------------------------------------------------
# 关键设置：告诉 CMake 你的 Qt 在哪里
# 注意：路径里的斜杠必须是 '/'，不能是 Windows 的 '\'
# -----------------------------------------------------------
set(CMAKE_PREFIX_PATH "D:/Qt/6.10.2/mingw_64") 

# 查找 Qt 的 Widgets 模块 (做界面用的)
find_package(Qt6 REQUIRED COMPONENTS Widgets Network)

# 添加你的源代码文件
add_executable(Z-Td 
    main.cpp 
    mainwindow.cpp 
    mainwindow.h 
    logo.rc
)
# 链接 Qt 库
target_link_libraries(Z-Td PRIVATE Qt6::Widgets Qt6::Network)

# 防止打开时后面跟着一个黑框框 (控制台)
set_target_properties(Z-Td PROPERTIES WIN32_EXECUTABLE ON)