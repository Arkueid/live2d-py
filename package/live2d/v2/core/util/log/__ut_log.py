import time

RESET = "\033[0m"
RED = "\033[31m"
GREEN = "\033[32m"
YELLOW = "\033[33m"
BLUE = "\033[34m"
MAGENTA = "\033[35m"
CYAN = "\033[36m"
WHITE = "\033[37m"

__enable = True


def setLogEnable(v: bool):
    global __enable
    __enable = v


def logEnable() -> bool:
    return __enable


def Debug(*args, **kwargs):
    if __enable:
        print(
            time.strftime(f"{BLUE}[DEBUG]"),
            *args,
            RESET,
            **kwargs
        )


def Info(*args, **kwargs):
    if __enable:
        print(
            time.strftime("[INFO] "),
            *args,
            **kwargs
        )


def Error(*args, **kwargs):
    if __enable:
        print(
            time.strftime(f"{RED}[ERROR]"),
            *args,
            RESET,
            **kwargs
        )
