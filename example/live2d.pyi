def InitializeCubism() -> None:
    """
    初始化 Cubism Framework
    """
    pass


def ReleaseCubism() -> None:
    """
    释放 Cubism Framework
    """
    pass


def InitializeGlew() -> None:
    """
    基于 Glew 实现的 live2d, 使用模型前应初始化 Glew
    """
    pass


def SetGLProperties() -> None:
    """
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    
    glEnable(GL_BLEND);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    """
    pass


def ClearBuffer() -> None:
    """
    glClearColor(0.0, 0.0, 0.0, 0.0)
        
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
    
    glClearDepth(1.0)
    """
    pass


class LAppModel:
    def __init__(self):
        pass
    
    def LoadAssets(self, dir: str, fileName: str) -> None:
        """
        加载 live2d 模型
        :param: dir 模型文件夹
        :param: fileName 模型json文件
        """
        pass
    
    def Update(self, ww: int, wh: int) -> None:
        """
        每帧调用，更新模型
        """
        pass
    
    def StartMotion(self, group: str, no: int, priority: int, onFinishedMotionHandler=None) -> None:
        pass
    
    def StartRandomMotion(self, group: str, priority: int, onFinishedMotionHandler=None) -> None:
        pass
    
    def SetExpression(self, expressionID: str) -> None:
        pass
    
    def SetRandomExpression(self) -> None:
        pass
    
    def HitTest(self, hitAreaName: str, x: float, y: float) -> str:
        pass
    
    def HasMocConsistencyFromFile(self, mocFileName: str) -> bool:
        pass
