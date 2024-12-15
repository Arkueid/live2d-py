import time


class UtSystem:
    USER_TIME_AUTO = -1
    userTimeMSec = USER_TIME_AUTO

    @staticmethod
    def isBigEndian():
        return True

    @staticmethod
    def wait(duration):
        try:
            start_time = UtSystem.getTimeMSec()
            while UtSystem.getTimeMSec() - start_time < duration:
                pass

        except Exception as e:
            print(e)

    @staticmethod
    def getUserTimeMSec():
        return UtSystem.getSystemTimeMSec() if (
                    UtSystem.userTimeMSec == UtSystem.USER_TIME_AUTO) else UtSystem.userTimeMSec

    @staticmethod
    def setUserTimeMSec(aH):
        UtSystem.userTimeMSec = aH

    @staticmethod
    def updateUserTimeMSec():
        UtSystem.userTimeMSec = UtSystem.getSystemTimeMSec()
        return UtSystem.userTimeMSec

    @staticmethod
    def getTimeMSec():
        return time.time() * 1000

    @staticmethod
    def getSystemTimeMSec():
        return time.time() * 1000

    @staticmethod
    def arraycopy(aM, aJ, aI, aL, aH):
        for aK in range(0, aH, 1):
            aI[aL + aK] = aM[aJ + aK]
