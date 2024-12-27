class StandardParams:
    ParamAngleX = "ParamAngleX"
    ParamAngleY = "ParamAngleY"
    ParamAngleZ = "ParamAngleZ"
    ParamEyeLOpen = "ParamEyeLOpen"
    ParamEyeLSmile = "ParamEyeLSmile"
    ParamEyeROpen = "ParamEyeROpen"
    ParamEyeRSmile = "ParamEyeRSmile"
    ParamEyeBallX = "ParamEyeBallX"
    ParamEyeBallY = "ParamEyeBallY"
    ParamEyeBallForm = "ParamEyeBallForm"
    ParamBrowLY = "ParamBrowLY"
    ParamBrowRY = "ParamBrowRY"
    ParamBrowLX = "ParamBrowLX"
    ParamBrowRX = "ParamBrowRX"
    ParamBrowLAngle = "ParamBrowLAngle"
    ParamBrowRAngle = "ParamBrowRAngle"
    ParamBrowLForm = "ParamBrowLForm"
    ParamBrowRForm = "ParamBrowRForm"
    ParamMouthForm = "ParamMouthForm"
    ParamMouthOpenY = "ParamMouthOpenY"
    ParamCheek = "ParamCheek"
    ParamBodyAngleX = "ParamBodyAngleX"
    ParamBodyAngleY = "ParamBodyAngleY"
    ParamBodyAngleZ = "ParamBodyAngleZ"
    ParamBreath = "ParamBreath"
    ParamArmLA = "ParamArmLA"
    ParamArmRA = "ParamArmRA"
    ParamArmLB = "ParamArmLB"
    ParamArmRB = "ParamArmRB"
    ParamHandL = "ParamHandL"
    ParamHandR = "ParamHandR"
    ParamHairFront = "ParamHairFront"
    ParamHairSide = "ParamHairSide"
    ParamHairBack = "ParamHairBack"
    ParamHairFluffy = "ParamHairFluffy"
    ParamShoulderY = "ParamShoulderY"
    ParamBustX = "ParamBustX"
    ParamBustY = "ParamBustY"
    ParamBaseX = "ParamBaseX"
    ParamBaseY = "ParamBaseY"


class Parameter:

    def __init__(self):
        self.id: str = ""
        self.type: int = 0
        self.value: float = 0
        self.max: float = 0
        self.min: float = 0
        self.default: float = 0
