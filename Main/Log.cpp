#include "Log.hpp"

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <time.h>
#include <stdarg.h>
#include <string.h>

#define LOG_INFO_CONSOLE_FORMAT "[INFO  %s] %s\n"
#define LOG_ERROR_CONSOLE_FORMAT "\033[31m[ERROR %s] %s\033[0m\n"
#define LOG_INFO_FILE_FORMAT "[INFO  %s] %s\n"
#define LOG_ERROR_FILE_FORMAT "[ERROR %s] %s\n"
#define LOG_INFO_OUTPUT "info.log"
#define LOG_ERROR_OUTPUT "error.log"

#define LOG_DEBUG_CONSOLE_FORMAT "\033[34m[DEBUG %s] %s\033[0m\n"
#define LOG_DEBUG_FILE_FORMAT "[DEBUG %s] %s\n"

#define TIME_BUFSIZE 20
#define MAX_LEVEL_HEADER_SIZE 32
#define MAX_BUFSIZE 1024
#define MAX_MSG_SIZE (MAX_BUFSIZE - MAX_LEVEL_HEADER_SIZE)

void current_time(char *buf)
{
    time_t rawtime;
    struct tm timeinfo;
    time(&rawtime);
    localtime_s(&timeinfo, &rawtime);

    strftime(buf, TIME_BUFSIZE, "%Y-%m-%d %H:%M:%S", &timeinfo);
}

static void WriteConsole(const char *level_fmt, ...)
{
    va_list args;
    va_start(args, level_fmt);

    char buffer[MAX_BUFSIZE];

    vsnprintf(buffer, MAX_BUFSIZE, level_fmt, args);

    va_end(args);

    printf(buffer);
}

static void WriteFile(const char *level_fmt, const char *file, ...)
{
    std::fstream f;

    f.open(file, std::ios::app);

    if (!f.is_open())
    {
        perror("file open error");
        exit(-1);
    }

    va_list args;
    va_start(args, file);

    char buffer[MAX_BUFSIZE];
    vsnprintf(buffer, MAX_BUFSIZE, level_fmt, args);

    f << buffer;

    va_end(args);

    f.close();
}

static bool _enable = true;

void setLogEnable(bool enable)
{
    _enable = enable;
}

void _Info(const char *fmt, ...)
{
    if (!_enable) return;

    char time_buf[TIME_BUFSIZE];
    current_time(time_buf);

    va_list args;
    va_start(args, fmt);

    char msg_buf[MAX_MSG_SIZE];
    vsnprintf(msg_buf, MAX_MSG_SIZE, fmt, args);
    va_end(args);

    WriteConsole(LOG_INFO_CONSOLE_FORMAT, time_buf, msg_buf);
}

void _InfoF(const char *file, const char *fmt, ...)
{
    if (!_enable) return;

    char time_buf[TIME_BUFSIZE];
    current_time(time_buf);
    va_list args;
    va_start(args, fmt);

    char msg_buf[MAX_MSG_SIZE];
    vsnprintf(msg_buf, MAX_MSG_SIZE, fmt, args);
    va_end(args);

    WriteFile(LOG_INFO_FILE_FORMAT, file, time_buf, msg_buf);
}

void _Error(const char *fmt, ...)
{
    if (!_enable) return;

    char time_buf[TIME_BUFSIZE];
    current_time(time_buf);
    va_list args;
    va_start(args, fmt);

    char msg_buf[MAX_MSG_SIZE];
    vsnprintf(msg_buf, MAX_MSG_SIZE, fmt, args);
    va_end(args);

    WriteConsole(LOG_ERROR_CONSOLE_FORMAT, time_buf, msg_buf);
}

void _ErrorF(const char *file, const char *fmt, ...)
{
    if (!_enable) return;

    char time_buf[TIME_BUFSIZE];
    current_time(time_buf);
    va_list args;
    va_start(args, fmt);

    char msg_buf[MAX_MSG_SIZE];
    vsnprintf(msg_buf, MAX_MSG_SIZE, fmt, args);
    va_end(args);

    WriteFile(LOG_ERROR_FILE_FORMAT, file, time_buf, msg_buf);
}

void _Debug(const char *fmt, ...)
{
    if (!_enable) return;

    char time_buf[TIME_BUFSIZE];
    current_time(time_buf);
    va_list args;
    va_start(args, fmt);

    char msg_buf[MAX_MSG_SIZE];
    vsnprintf(msg_buf, MAX_MSG_SIZE, fmt, args);
    va_end(args);

    WriteConsole(LOG_DEBUG_CONSOLE_FORMAT, time_buf, msg_buf);
}

void _DebugF(const char *file, const char *fmt, ...)
{
    if (!_enable) return;

    char time_buf[TIME_BUFSIZE];
    current_time(time_buf);
    va_list args;
    va_start(args, fmt);

    char msg_buf[MAX_MSG_SIZE];
    vsnprintf(msg_buf, MAX_MSG_SIZE, fmt, args);
    va_end(args);

    WriteFile(LOG_DEBUG_FILE_FORMAT, file, time_buf, msg_buf);
}
