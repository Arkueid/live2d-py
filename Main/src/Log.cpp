#include <ctime>
#include <cstdio>
#include <stdarg.h>

bool live2dLogEnable = true;

static char buffer[20];

const char* currentTime()
{
    // 2024-11-07 14:05:06
    time_t t = time(nullptr);
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localtime(&t));
    return buffer;
}

void Debug(const char *fmt, ...)
{
    if (live2dLogEnable)
    {
        printf("\033[34m[DEBUG %s] ", currentTime());
        va_list args;
        va_start(args, fmt);
        vfprintf(stdout, fmt, args);
        va_end(args);
        printf("\033[0m\n");
    }
}

void Info(const char *fmt, ...)
{
    if (live2dLogEnable)
    {
        printf("[INFO %s] ", currentTime());
        va_list args;
        va_start(args, fmt);
        vfprintf(stdout, fmt, args);
        va_end(args);
        printf("\n");
    }
}

void Warn(const char *fmt, ...)
{
    if (live2dLogEnable)
    {
        printf("\033[33m[WARN %s] ", currentTime());
        va_list args;
        va_start(args, fmt);
        vfprintf(stdout, fmt, args);
        va_end(args);
        printf("\033[0m\n");
    }
}

void Error(const char *fmt, ...)
{
    if (live2dLogEnable)
    {
        printf("\033[31m[ERROR %s] ", currentTime());
        va_list args;
        va_start(args, fmt);
        vfprintf(stdout, fmt, args);
        va_end(args);
        printf("\033[0m\n");
    }
}
