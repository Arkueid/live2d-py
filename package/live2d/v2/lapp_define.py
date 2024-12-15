class MotionPriority:
    NONE = 0
    IDLE = 1
    NORMAL = 2
    FORCE = 3


class MotionGroup:
    IDLE = "idle"
    TAP_BODY = "tap_body"
    FLICK_HEAD = "flick_head"
    PINCH_IN = "pinch_in"
    PINCH_OUT = "pinch_out"
    SHAKE = "shake"


class HitArea:
    HEAD = "head"
    BODY = "body"
