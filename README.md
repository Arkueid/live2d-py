<div align="center" style="padding-top: 10px">
    <img src="./docs/koe.svg" alt="koe" width="50px" height="50px">
    <img src="./docs/logo.svg" alt="live2d-py" height="80px">
</div>

<p align="center" style="font-family: 'Roboto', sans-serif; font-size: 1em; color: #555;">
    <img title="Windows Distro" src="https://github.com/Arkueid/live2d-py/actions/workflows/build-windows.yml/badge.svg">
    <br>
    <img title="MacOS Distro" src="https://github.com/Arkueid/live2d-py/actions/workflows/build-macos.yml/badge.svg">
    <br>
    <img title="Release Version" src="https://img.shields.io/github/v/release/Arkueid/live2d-py" alt="Docker Build Version" style="margin: 0 10px;">
    <img title="Python Version" src="https://img.shields.io/badge/python-3.8+-blue" alt="Python Version" style="margin: 0 10px;">
    <img title="CMake" src="https://img.shields.io/badge/CMake-3.16+-orange" alt="CMake" style="margin: 0 10px;">
    <img title="C++" src="https://img.shields.io/badge/C%2B%2B-17-yellow" alt="C++17" style="margin: 0 10px;">
    <br>
    <a href="./README.en.md">English README</a>
</p>

使用 Python 直接加载和操作 Live2D 模型，不通过 Web Engine 等间接手段进行渲染。

基于 Python C++ API 对 Live2D Native SDK (C++) 进行了封装。理论上，只要配置好 OpenGL 上下文，可在 Python 中将 live2d 绘制在任何基于 OpenGL 的窗口。

代码使用示例：[package](./package/)

详细使用文档：[Wiki](https://github.com/Arkueid/live2d-py/wiki)

修改和开发：[CONTRIBUTING](./CONTRIBUTING.md)

## 兼容UI库

理论上兼容所有能使用 OpenGL 进行绘制的UI库： Pygame / PyQt5 / PySide2 / PySide6 / GLFW / pyopengltk/ FreeGlut / Qfluentwidgets ...

## 支持功能

* 加载模型：**Cubism 2.1** 和 **Cubism 3.0** 及以上版本
* 视线跟踪
* 点击交互
* 动作播放回调
* 口型同步
* 模型各部分参数控制
* 各部件透明度控制
* 精确到部件的点击检测

## 兼容性

### Python 版本
Python 版本支持：从 live2d-py 0.3.2 开始使用 Python C Limited API，理论上兼容 Python 3.2 以上的所有版本。

### Cubism Live2D 版本

| `live2d-py` | 支持的live2d模型        | 实现                    | 支持的Python版本                           | 支持平台                     |
|-------------|--------------------|-----------------------|---------------------------------------|--------------------------|
| `live2d.v2` | Cubism 2.1 以及更早的版本 | 纯 Python 实现           | 支持 `32` / `64` 位，支持`Python 3.0` 及以上版本 | Winodws、Linux、MacOS（理论上） |                                                       |
| `live2d.v3` | Cubism 3.0 及以上版本   | Python C Extension 封装 | 支持 `32` / `64` 位，支持`Python 3.2` 及以上版本 | Windows、Linux            |

注：

* `live2d.v2` 由 Cubism Web SDK 转写为纯 Python，尚未使用 numpy 等优化的库，性能有待提升。
* Cubism 2.X 导出的模型：文件名格式常为 `XXX.moc`，`XXX.model.json`，`XXX.mtn`
* Cubism 3.0 及以上导出的模型：文件名格式常为 `XXX.moc3`，`XXX.model3.json`, `XXX.motion3.json`

## 使用说明

1. 将仓库中的 package/live2d 直接作为 Python 模块集成到你的项目中即可。


2. 在 [Release](https://github.com/Arkueid/live2d-py/releases/latest) 中下载对应版本的 `whl` 文件并安装（推荐）

```shell
pip install live2d_py-0.X.X-cpXXX-cpXXX-win32.whl
```

3. 通过 [PyPI](https://pypi.org/project/live2d-py/) 安装

```shell
pip install live2d-py
```

4. 从源码构建，参考 [安装#源码构建](https://github.com/Arkueid/live2d-py/wiki/%E5%AE%89%E8%A3%85#%E6%BA%90%E7%A0%81%E6%9E%84%E5%BB%BA)

## 简易面部动捕示例

源码见 [main_facial_bind_mediapipe.py](./package/main_facial_bind.py)

![面捕-期末周破防](./docs/video_test.gif)

## 贡献

特别感谢 [@96bearli]，[@Ovizro], [@AnyaCoder], [@jahtim], [@Honghe], [@RobertMeow] 为本项目提供的帮助和支持。

[@96bearli]: https://github.com/96bearli

[@Ovizro]: https://github.com/Ovizro

[@AnyaCoder]: https://github.com/AnyaCoder

[@jahtim]: https://github.com/jahtim

[@Honghe]: https://github.com/Honghe

[@RobertMeow]: https://github.com/RobertMeow