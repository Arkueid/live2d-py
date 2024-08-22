#include "Log.hpp"

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <time.h>
#include <stdarg.h>
#include <string.h>
#include <mutex>

#define LOG_INFO_CONSOLE_FORMAT "[INFO  %s] %s\n"
#define LOG_ERROR_CONSOLE_FORMAT "\033[31m[ERROR %s] %s\033[0m\n"
#define LOG_DEBUG_CONSOLE_FORMAT "\033[34m[DEBUG %s] %s\033[0m\n"

#define TIME_BUFSIZE 20
#define MAX_LEVEL_HEADER_SIZE 32
#define MAX_BUFSIZE 1024
#define MAX_MSG_SIZE (MAX_BUFSIZE - MAX_LEVEL_HEADER_SIZE)

static bool enable = true;

static std::mutex log_lock;

void setLogEnable(bool on)
{
    enable = on;
}

void current_time(char *buf)
{
    time_t rawtime;
    struct tm *timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buf, TIME_BUFSIZE, "%Y-%m-%d %H:%M:%S", timeinfo);
}

static void WriteConsole(const char *level_fmt, ...)
{
    va_list args;
    va_start(args, level_fmt);

    char buffer[MAX_BUFSIZE];

    std::vsnprintf(buffer, MAX_BUFSIZE, level_fmt, args);

    va_end(args);

    printf(buffer);
}

void _LOG(const int level, const char *fmt, ...)
{
    if (!enable)
        return;

    char time_buf[TIME_BUFSIZE];
    current_time(time_buf);

    va_list args;
    va_start(args, fmt);

    char msg_buf[MAX_MSG_SIZE];
    std::vsnprintf(msg_buf, MAX_MSG_SIZE, fmt, args);
    va_end(args);

    const char *format;
    if (level == LogLevel::DEBUG)
    {
        format = LOG_DEBUG_CONSOLE_FORMAT;
    }
    else if (level == LogLevel::INFO)
    {
        format = LOG_INFO_CONSOLE_FORMAT;
    }
    else if (level == LogLevel::ERROR)
    {
        format = LOG_ERROR_CONSOLE_FORMAT;
    }
    WriteConsole(format, time_buf, msg_buf);
}
