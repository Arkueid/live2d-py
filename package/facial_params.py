class Params:
    """
    面部参数
    """
    EyeLOpen: float = 1.0
    EyeROpen: float = 1.0
    MouthOpenY: float = 0.0
    MouthForm: float = 0.0
    AngleX: float = 0.0
    AngleY: float = 0.0
    AngleZ: float = 0.0
    BodyAngleX: float = 0.0
    BodyAngleY: float = 0.0
    BodyAngleZ: float = 0.0
    # 前一帧参数存储
    prev_EyeLOpen = 0
    prev_EyeROpen = 0
    prev_MouthOpenY = 0
    prev_AngleX = 0
    prev_AngleY = 0
    prev_AngleZ = 0
    prev_BodyAngleX = 0
    # 滤波因子
    smooth_factor = 0.9  # 0~1之间，值越大越平滑画面迟滞越明显，值越小抖动越明显画面变化越灵敏

    def smooth(self, current_value, prev_value):
        return self.smooth_factor * prev_value + (1 - self.smooth_factor) * current_value

    def update_params(self, new_params):
        self.EyeLOpen = self.smooth(new_params.EyeLOpen, self.prev_EyeLOpen)
        self.prev_EyeLOpen = self.EyeLOpen

        self.EyeROpen = self.smooth(new_params.EyeROpen, self.prev_EyeROpen)
        self.prev_EyeROpen = self.EyeROpen

        self.MouthOpenY = self.smooth(new_params.MouthOpenY, self.prev_MouthOpenY)
        self.prev_MouthOpenY = self.MouthOpenY

        self.AngleX = self.smooth(new_params.AngleX, self.prev_AngleX)
        self.prev_AngleX = self.AngleX

        self.AngleY = self.smooth(new_params.AngleY, self.prev_AngleY)
        self.prev_AngleY = self.AngleY

        self.AngleZ = self.smooth(new_params.AngleZ, self.prev_AngleZ)
        self.prev_AngleZ = self.AngleZ

        self.BodyAngleX = self.smooth(new_params.BodyAngleX, self.prev_BodyAngleX)
        self.prev_BodyAngleX = self.BodyAngleX