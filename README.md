<div align="center">
    <h1>live2d-py</h1>
</div>

<p align="center" style="font-family: 'Roboto', sans-serif; font-size: 1em; color: #555;">
    <img title="Docker Build Version" src="https://img.shields.io/github/v/release/Arkueid/live2d-py" alt="Docker Build Version" style="margin: 0 10px;">
    <img title="Python Version" src="https://img.shields.io/badge/python-3.12-blue" alt="Python Version" style="margin: 0 10px;">
    <img title="Python Version" src="https://img.shields.io/badge/python-3.10.11-blue" alt="Python Version" style="margin: 0 10px;">
    <img title="CMake" src="https://img.shields.io/badge/CMake-3.13-orange" alt="CMake" style="margin: 0 10px;">
    <img title="C++" src="https://img.shields.io/badge/C%2B%2B-17-yellow" alt="C++17" style="margin: 0 10px;">
</p>

使用 Python 直接加载和操作 Live2D 模型，不通过 Web Engine 等间接手段进行渲染，提供更高的自由度和拓展性。

基于 Python C++ API 对 Live2D Native SDK (C++) 进行了封装。理论上，只要配置好 OpenGL 上下文，可在 Python 中将 live2d 绘制在任何基于 OpenGL 的窗口。

详细使用文档：https://arkueid.github.io/live2d-py-docs/

## 支持UI库
理论上支持所有能使用 OpenGL 进行绘制的UI库：PyQt5 / PySide2 / PySide6 / GLFW / FreeGlut / Qfluentwidgets ...

## 基本操作
* 加载模型
* 鼠标拖拽视线
* 鼠标点击触发动作
* 动作播放回调函数
* 口型同步
* 模型各部分参数控制

## 平台支持

| `live2d-py` | 支持的live2d模型            | 支持的Python版本                                                    | 支持平台          |
|-------------|------------------------|----------------------------------------------------------------|---------------|
| `live2d.v2` | Cubism 2.1 以及更早的版本 | 仅 32 位，支持`Python 3.0` 及以上版本，但除 `Python 3.10.11` 外需要自行编译        | Windows       |
| `live2d.v3` | Cubism 3.0 及以上版本  | 支持 `32` / `64` 位，支持`Python 3.0` 及以上版本，但除 `Python 3.12` 外需要自行编译 | Windows、Linux |

注：
* Cubism 2.X 导出的模型：文件名格式常为 `XXX.moc`，`XXX.model.json`，`XXX.mtn`
* Cubism 3.0 及以上导出的模型：文件名格式常为 `XXX.moc3`，`XXX.model3.json`, `XXX.motion3.json` 
* 对于 Cubism 2.0 模型，网络上能找到的现存 live2d opengl 静态库只有 32 位，因此只能使用 32 位 Python 解释器加载。

## 安装方式

通过 PyPi 安装
```shell
pip install live2d-py
```

在发行版中下载源码进行构建安装
```shell
pip install live2d_py-0.2.2.tar.gz
```

在发行版中下载对应版本的 `whl` 文件并安装
```shell
pip install live2d_py-0.2.2-cp310-cp310-win32.whl
```

克隆本仓库，自行编译构建，参考 [安装/编译](https://arkueid.github.io/live2d-py-docs/%E4%BD%BF%E7%94%A8%E8%AF%B4%E6%98%8E/%E5%AE%89%E8%A3%85.html#%E7%BC%96%E8%AF%91)


## 简易面部动捕示例
源码见 [main_facial_bind_mediapipe.py](./package/main_facial_bind_mediapipe.py)  

![期末周破防](./docs/video_test.gif)

![简易动捕](./docs/facial_capture.gif)

## 基于 live2d-py + qfluentwidgets 实现的桌面应用预览

见 [live2d-desktop](https://github.com/Arkueid/Live2DMascot)

![alt](./docs/2.png)
