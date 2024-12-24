import time

RESET = "\033[0m"
RED = "\033[31m"
GREEN = "\033[32m"
YELLOW = "\033[33m"
BLUE = "\033[34m"
MAGENTA = "\033[35m"
CYAN = "\033[36m"
WHITE = "\033[37m"


def Debug(*args, **kwargs):
    print(
        time.strftime(f"{BLUE}[DEBUG  %Y-%m-%d %H:%M:%S]", time.localtime(time.time())),
        *args,
        RESET,
        **kwargs
    )


def Info(*args, **kwargs):
    print(
        time.strftime("[INFO  %Y-%m-%d %H:%M:%S]", time.localtime(time.time())),
        *args,
        **kwargs
    )


def Error(*args, **kwargs):
    print(
        time.strftime(f"{RED}[ERROR  %Y-%m-%d %H:%M:%S]", time.localtime(time.time())),
        *args,
        RESET,
        **kwargs
    )
