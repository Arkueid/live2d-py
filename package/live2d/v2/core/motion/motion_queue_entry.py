from ..util import UtSystem


class MotionQueueEntry:
    MQ_NO = 0

    def __init__(self):
        self.motion = None
        self.available = True
        self.finished = False
        self.startTimeMSec = -1
        self.fadeInStartTimeMSec = -1
        self.endTimeMSec = -1
        self.mqNo = MotionQueueEntry.MQ_NO
        MotionQueueEntry.MQ_NO += 1

    def isFinished(self):
        return self.finished

    def startFadeOut(self, fadeOutMSec):
        ct = UtSystem.getUserTimeMSec()
        new_end_time_m_sec = ct + fadeOutMSec
        if self.endTimeMSec < 0 or new_end_time_m_sec < self.endTimeMSec:
            self.endTimeMSec = new_end_time_m_sec
