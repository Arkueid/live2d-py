import subprocess


def create_ffmpeg_task(width: int, height: int, output_file: str):
    ffmpeg_command = [
        "ffmpeg",
        "-y",
        "-f", "rawvideo",
        "-vcodec", "rawvideo",
        "-pix_fmt", "rgba",
        "-s", f"{width}x{height}",
        "-i", "-", # 使用 stdin 读取
        "-c:v", "prores_ks", # mov格式 编码
        "-profile:v", "4",
        "-pix_fmt", "yuv420p",
        output_file
    ]

    return subprocess.Popen(ffmpeg_command, stdin=subprocess.PIPE)

