class Live2DFramework:
    __platformManager = None

    @staticmethod
    def getPlatformManager():
        return Live2DFramework.__platformManager

    @staticmethod
    def setPlatformManager(platformManager):
        Live2DFramework.__platformManager = platformManager
