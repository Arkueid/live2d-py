#pragma once
#include <ctime>

extern bool live2dLogEnable;


inline const char* currentTime()
{
    // 2024-11-07 14:05:06
    static char buffer[20];
    time_t t = time(nullptr);
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localtime(&t));
    return buffer;
}

#define Debug(fmt, ...) do { \
    if (live2dLogEnable) { \
        printf("\033[34m[DEBUG %s] ", currentTime());\
        printf(fmt, ##__VA_ARGS__); \
        printf("\033[0m\n");\
    } \
} while (0)

#define Info(fmt, ...) do { \
    if (live2dLogEnable) { \
        printf("[INFO %s] ", currentTime());\
        printf(fmt, ##__VA_ARGS__); \
        printf("\n");\
    } \
} while (0)

#define Warn(fmt, ...) do { \
    if (live2dLogEnable) { \
        printf("\033[33m[WARN %s] ", currentTime());\
        printf(fmt, ##__VA_ARGS__); \
        printf("\033[0m\n");\
    } \
} while (0)

#define Error(fmt, ...) do { \
    if (live2dLogEnable) { \
        printf("\033[31m[ERROR %s] ", currentTime());\
        printf(fmt, ##__VA_ARGS__); \
        printf("\033[0m\n");\
    } \
} while (0)
