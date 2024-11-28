#include <ctime>

bool live2dLogEnable = true;

const char* currentTime()
{
    // 2024-11-07 14:05:06
    static char buffer[20];
    time_t t = time(nullptr);
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localtime(&t));
    return buffer;
}
