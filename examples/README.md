# Live2D Python Examples 使用指南

本文档将指导您如何将示例代码迁移到自己的工程中使用。

## 环境要求

- Python 3.7+
- live2d-python 包
- OpenGL 相关依赖

## 快速开始

1. 首先确保您已安装必要的依赖:
```
pip install -r requirements.txt
```
包括按照主README.md里面的步骤安装live2d-py

2. 从示例代码中复制所需文件:
- `resources.py`
- `模型文件夹` (根据您的模型选择)

3. 确保您的项目结构如下:
```
your_project/
│
├── main.py
├── resources.py
└── Resources
```

4. 复制示例代码并替换main.py,运行.