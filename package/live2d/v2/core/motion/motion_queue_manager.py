from ..motion import MotionQueueEntry


class MotionQueueManager:

    def __init__(self):
        self.motions = []

    def startMotion(self, aJ, aI):
        count = len(self.motions)
        for i in range(0, count, 1):
            ent = self.motions[i]
            if ent is None:
                continue

            ent.startFadeOut(ent.motion.getFadeOut())

        if aJ is None:
            return -1

        ent = MotionQueueEntry()
        ent.motion = aJ
        self.motions.append(ent)
        nr = ent.mqNo

        return nr

    def updateParam(self, aJ):
        updated = False
        i = 0
        size = len(self.motions)
        while i < size:
            ent = self.motions[i]
            if ent is None:
                self.motions.pop(i)
                size -= 1
                continue

            mtn = ent.motion
            if mtn is None:
                self.motions.pop(i)
                size -= 1
                continue

            mtn.updateParam(aJ, ent)
            updated = True
            if ent.isFinished():
                ent = self.motions.pop(i)
                size -= 1
                i -= 1
            else:
                pass
            i += 1

        return updated

    def isFinished(self, nr=None):
        if nr is not None:  # is the motion finished?
            for i in range(0, len(self.motions), 1):
                ent = self.motions[i]
                if ent is None:
                    continue

                if ent.mqNo == nr and not ent.isFinished():
                    return False

            return True
        else:  # are all the motions finished?
            i = 0
            size = len(self.motions)
            while i < size:
                ent = self.motions[i]
                if ent is None:
                    self.motions.pop(i)
                    size -= 1
                    continue

                aH = ent.motion
                if aH is None:
                    self.motions.pop(i)
                    size -= 1
                    continue

                if not ent.isFinished():
                    return False
                i += 1

            return True

    def stopAllMotions(self):
        size = len(self.motions)
        i = 0
        while i < size:
            ent = self.motions[i]
            if ent is None:
                self.motions.pop(i)
                size -= 1
                continue

            mtn = ent.motion
            if mtn is None:
                self.motions.pop(i)
                size -= 1
                continue

            self.motions.pop(i)
            size -= 1
