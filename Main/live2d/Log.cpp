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

bool live2dLogEnable = true;

void current_time(char *buf)
{
    time_t rawtime;
    struct tm *timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buf, TIME_BUFSIZE, "%Y-%m-%d %H:%M:%S", timeinfo);
}

void _LOG(const int level, const char *fmt, ...)
{
    if (!live2dLogEnable)
        return;

    char time_buf[TIME_BUFSIZE];
    current_time(time_buf);

    va_list args;
    va_start(args, fmt);

    int msgSize = std::vsnprintf(NULL, 0, fmt, args) + 1;

    char *msgBuf = (char *)malloc(msgSize * sizeof(char));
    std::vsnprintf(msgBuf, msgSize, fmt, args);
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
    else if (level == LogLevel::_ERROR)
    {
        format = LOG_ERROR_CONSOLE_FORMAT;
    }

    printf(format, time_buf, msgBuf);

    free(msgBuf);
}
