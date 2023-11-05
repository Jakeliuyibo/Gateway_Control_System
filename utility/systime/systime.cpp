#include <time.h>
#include <string.h>

#include "systime.h"

using namespace utility;

// 获取系统时间
std::string utility::getSystime()
{
    time_t ticks = time(NULL);
    struct tm *ptm = localtime(&ticks);
    char timestamp[32];
    memset(timestamp, 0, sizeof(timestamp));
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", ptm);
    return timestamp;
}

// 获取系统时间，文件名格式
std::string utility::getSystimeByFilenameFormat()
{
    time_t ticks = time(NULL);
    struct tm *ptm = localtime(&ticks);
    char timestamp[32];
    memset(timestamp, 0, sizeof(timestamp));
    strftime(timestamp, sizeof(timestamp), "%Y_%m_%d_%H_%M_%S", ptm);
    return timestamp;
}
