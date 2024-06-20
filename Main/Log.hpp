#pragma once

void setLogEnable(bool enable);
void _Info(const char *fmt, ...);
void _InfoF(const char *file, const char *fmt, ...);
void _Error(const char *fmt, ...);
void _ErrorF(const char *file, const char *fmt, ...);
void _Debug(const char *fmt, ...);
void _DebugF(const char *file, const char *fmt, ...);

#ifdef LOG_MODE_RELEASE
#define Debug(fmt, ...) ((void)0)
#define Info(fmt, ...) ((void)0)
#define Error(fmt, ...) _ErrorF("error.log", fmt, ##__VA_ARGS__)
#else
#define Debug(fmt, ...) _Debug(fmt, ##__VA_ARGS__)
#define Info(fmt, ...) _Info(fmt, ##__VA_ARGS__)
#define Error(fmt, ...) _Error(fmt, ##__VA_ARGS__)
#endif