#pragma once

enum LogLevel
{
    INFO,
    DEBUG,
    _ERROR
};


void _LOG(const int format, const char *fmt, ...);

#define Debug(fmt, ...) _LOG(LogLevel::DEBUG, fmt, ##__VA_ARGS__)
#define Info(fmt, ...) _LOG(LogLevel::INFO, fmt, ##__VA_ARGS__)
#define Error(fmt, ...) _LOG(LogLevel::_ERROR, fmt, ##__VA_ARGS__)
