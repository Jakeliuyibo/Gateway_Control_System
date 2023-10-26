#pragma once

#include <iostream>
#include <string>

using namespace std;

namespace utility {
class SysTime
{
public:
    SysTime()
    {
    }
    ~SysTime()
    {
    }
    // 获取系统时间
    string getSystime();
    // 获取系统时间，文件名格式
    string getSystimeByFilenameFormat();
};


}