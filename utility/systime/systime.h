#pragma once

#include <string>

namespace utility 
{
    // 获取系统时间
    std::string getSystime();
    // 获取系统时间
    std::string getSystimeUs();
    // 获取系统时间，文件名格式
    std::string getSystimeByFilenameFormat();
}