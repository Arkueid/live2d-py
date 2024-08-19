import time


def Info(msg: str):
    print(
        time.strftime(
            f"[INFO  %Y-%m-%d %H:%M:%S] {msg}",
            time.localtime(time.time())))