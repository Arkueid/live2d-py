# 更新内容

## 2024/8/19
添加:
* 日志打印、口型同步工具（需要`numpy`）：`live2d.log`，`live2d.lipsync`
* 修改接口：`CalcParameters` => `Update`，`Update` => `Draw`
* 标准参数：`live2d.v3.params.StandardParams`

移除：
* 内置口型同步功能：`LAppModel.SetLipSyncEnable`，`LAppModel.SetLipSyncN`

其他：
* 将v3版本的更新同步到v2上


新包结构如下：
```
package\live2d
|-- utils
|   |
|   |-- lipsync.py  # 口型同步工具
|   `-- log.py      # 日志工具
`-- v3
    |-- __init__.py
    |-- live2d.pyd  # 动态库/封装c++函数
    |-- live2d.pyi  # 接口&文档
    `-- params.py   # live2d 标准参数id
```

绘制步骤的接口名更改如下：
```python
live2d.clearBuffer()

# 初始化呼吸、动作、姿势、表情、各部分透明度等必要的参数值（如果对应的功能开启
model.Update() # CalcParameters() 更改为 Update()

# 在初始化的基础上修改参数（具体用法参考 live2d.pyi 文件
# 直接赋值
model.SetParameterValue("ParamAngleX", 15, 1.)
# 在原值基础上添加
model.AddParameterValue("ParamAngleX", 15)

# 执行绘制
model.Draw() # 原来的 Update() 更改为 Draw()
```

## 2024/8/17
添加：  
* 口型同步、自动呼吸、自动眨眼开关
* 动作参数控制，`LAppModel.SetParameterValue`，`LAppModel.AddParameterValue`
* 简易面部动捕示例

修复：
* v3 版本播放动作时没有对应的动作文件导致崩溃

可用参数见 Live2D 官方文档：https://docs.live2d.com/en/cubism-editor-manual/standard-parameter-list/
