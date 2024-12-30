from typing import TYPE_CHECKING, Optional

if TYPE_CHECKING:
    from ..platform_manager import PlatformManager

class Live2DFramework:
    __platformManager: Optional['PlatformManager'] = None

    @staticmethod
    def getPlatformManager() -> Optional['PlatformManager']:
        return Live2DFramework.__platformManager

    @staticmethod
    def setPlatformManager(platformManager: Optional['PlatformManager']) -> None:
        Live2DFramework.__platformManager = platformManager
