[python c api]: https://docs.python.org/3/c-api/index.html

[Core api 文档]: https://docs.live2d.com/en/cubism-sdk-manual/cubism-core-api-reference/

# 开发说明

在修改或参与开发这个项目之前，需要了解一些 CMake、 Cubism Native SDK、Live2D 模型文件结构的相关知识。

深入开发需要少量图形学基础和 OpenGL 基础，了解部分 Python C API。

* Cubism 相关部分可以查阅官方文档，这里推荐官方的 [Core api 文档]（可以下载pdf），可以对整个 Live2D 绘制流程有一个整体把握。
* [Python c api]

## 项目结构

整个项目由两个部分构成：live2d.v3 和 live2d.v2。

live2d.v2 完全采用 python 实现。代码是通过工具对 live2d.min.js 反混淆、转 Python 生成，并辅以手动修复💦。在 live2d.min.js 的功能基础上，额外增加了点击部件的精确检测、部件颜色设置等功能。性能欠佳，~~因为保留了一部分 javascript 特性~~。 

live2d.v3 由 [python c api] 进行封装。

## live2d.v3 构成

live2d.v3 使用 CMake 生成 Python 可以调用的动态库。Python 包的管理支持使用 setuptools，见 setup.py。

live2d.v3 分为：Core、Framework、Main、Wrapper 四个模块。

### Core

Cubism Native Core，包括一个头文件`.h`和若干平台对应的静态库。用于读取 Cubism 3.0 及以上 live2d 模型的 `.moc3` 文件。

### Framework
Cubism Native Framework，在 Core 层上的功能库，比如json文件读取、物理计算、图形绘制等）。

> 上面两个模块，在官方发布新版本后可以直接替换，几乎不需要做修改（实际上修改，4行左右）。

### Main
对应原来 Cubism Native SDK 的应用层，对其进行了精简。Main 在 Framework 基础上实现了一个可以绘制的 `LAppModel` cpp 类，增改功能主要是修改 `LAppModel.cpp` 中定义的类。Main 中的其他文件几乎很少改动。

Framework 和 Main 会分别生成自己的静态库。

> 上面的三个模块和 Python 无关，也可以用于绑定其他编程语言。

### Wrapper 
即 `LAppModelWrapper.cpp`，将 Main 中实现的 `LAppModel` cpp 类封装为 Python 模块，是整个项目中唯一引入 Python 相关依赖的位置。

编译过程：Framework 模块编译生成 Framework.lib -> Main 模块生成 live2d.lib -> Wrapper 模块生成 LAppModelWrapper.dll 并重命名为 live2d.pyd。

## 待完成
* Macos 上的编译
* 定制 Linux 平台的 workflow
* live2d.v2 的性能提升
