#include <time.h>
#include <string.h>
#include <chrono>
#include <sstream>
#include <iomanip>

#include "systime.hpp"

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

std::string utility::getSystimeUs()
{
    auto now = std::chrono::system_clock::now();
    std::time_t currentTime = std::chrono::system_clock::to_time_t(now);
    std::tm* localTime = std::localtime(&currentTime);
    
    // 获取当前微妙
    auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count() % 1000000;
    
    // 创建一个字符串流
    std::stringstream ss;
    ss << std::put_time(localTime, "%Y-%m-%d %H:%M:%S");
    ss << "." << std::setw(6) << std::setfill('0') << microseconds;
    return ss.str();
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
