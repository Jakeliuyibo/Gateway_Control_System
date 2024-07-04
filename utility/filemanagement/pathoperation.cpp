#include "filemanagement.hpp"

#include "logger.hpp"

namespace utility
{

// 拼接路径
DirNameType SplicingDirWithAppend(const std::initializer_list<DirNameType> dirNames)
{
    DirType dir;
    for(auto dirName : dirNames)
    {
        DirType path(dirName);
        dir /= path;
    }
    return dir.string();
}

DirNameType SplicingDirWithConcat(const std::initializer_list<DirNameType> dirNames)
{
    DirType dir;
    for(auto dirName : dirNames)
    {
        DirType path(dirName);
        dir += path;
    }
    return dir.string();
}

// 获取当前路径
DirNameType GetCurrentDirName()
{
    DirType dir(std::filesystem::current_path());
    return dir.string();
}

// 解析路径名 
DirNameType GetParentDirName(const DirNameType& dirName)
{
    DirType dir(dirName);
    return dir.parent_path();
}

// 路径是否存在
bool IsDirExisted(const DirNameType& dirName)
{
    DirType dir(dirName);
    return std::filesystem::exists(dir);
}

// 路径是否为目录
bool IsDirectory(const DirNameType& dirName)
{
    DirType dir(dirName);
    return IsDirExisted(dirName) && std::filesystem::is_directory(dir);
}

// 创建目录
bool CreateDirectory(const DirNameType& dirName)
{
    DirType dir(dirName);
    return std::filesystem::create_directories(dir);
}

// 删除目录
bool RemoveDirectory(const DirNameType& dirName)
{
    DirType dir(dirName);
    return std::filesystem::remove_all(dir);
}











};


