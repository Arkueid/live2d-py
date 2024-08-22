#pragma once

enum LogLevel
{
    INFO,
    DEBUG,
    ERROR
};


void setLogEnable(bool on);
void _LOG(const int format, const char *fmt, ...);

#define Debug(fmt, ...) _LOG(LogLevel::DEBUG, fmt, ##__VA_ARGS__)
#define Info(fmt, ...) _LOG(LogLevel::INFO, fmt, ##__VA_ARGS__)
#define Error(fmt, ...) _LOG(LogLevel::ERROR, fmt, ##__VA_ARGS__)
