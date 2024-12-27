#pragma once

extern bool live2dLogEnable;

const char* currentTime();

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
