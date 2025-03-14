#include "Log.hpp"
#include <ctime>
#include <cstdio>
#include <cstdarg>

bool live2dLogEnable = true;

void Debug(const char *fmt, ...)
{
    if (live2dLogEnable)
    {
        printf("\033[34m[DEBUG] ");
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
        printf("[INFO]  ");
        va_list args;
        va_start(args, fmt);
        vfprintf(stdout, fmt, args);
        va_end(args);
        printf("\n");
    }
}

void Warn(const char *fmt, ...)
{
    printf("\033[33m[WARN]  ");
    va_list args;
    va_start(args, fmt);
    vfprintf(stdout, fmt, args);
    va_end(args);
    printf("\033[0m\n");
}

void Error(const char *fmt, ...)
{
    printf("\033[31m[ERROR] ");
    va_list args;
    va_start(args, fmt);
    vfprintf(stdout, fmt, args);
    va_end(args);
    printf("\033[0m\n");
}