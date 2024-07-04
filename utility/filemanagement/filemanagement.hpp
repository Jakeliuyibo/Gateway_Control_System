#pragma once

#include <iostream>
#include <filesystem>
#include <string>
#include <fstream>
#include <ostream>

namespace utility
{
    using DirType = std::filesystem::path;
    using DirNameType = std::string;
    using FileNameType = std::string;
    using FileSizeType = std::size_t;

    /* 路径相关操作 */
    // 拼接路径
    DirNameType SplicingDirWithAppend(const std::initializer_list<DirNameType> dirNames);
    DirNameType SplicingDirWithConcat(const std::initializer_list<DirNameType> dirNames);
    // 获取当前路径
    DirNameType GetCurrentDirName();
    // 解析路径名 
    DirNameType GetParentDirName(const DirNameType& dirName);
    // 路径是否存在
    bool IsDirExisted(const DirNameType& dirName);
    // 路径是否为目录
    bool IsDirectory(const DirNameType& dirName);
    // 创建目录
    bool CreateDirectory(const DirNameType& dirName);
    // 删除目录
    bool RemoveDirectory(const DirNameType& dirName);
    // 拷贝目录
    bool CopyDirectory(const DirNameType& oldDirName, const DirNameType& newDirName);
    // 重命名目录
    bool RenameDirectory(const DirNameType& oldDirName, const DirNameType& newDirName);

    /* 文件相关操作 */
    // 文件是否存在
    bool IsFileExisted(const DirNameType& dirName);
    // 解析文件名
    FileNameType GetFileName(const DirNameType& dirName);
    // 解析文件扩展名
    FileNameType GetFileExtensionName(const DirNameType& dirName);
    // 修改文件扩展名
    bool ModifyFileExtensionName(DirNameType& dirName, const FileNameType& modifyExtensionName);
    // 获取文件大小
    FileSizeType GetFileSize(const DirNameType& dirName);
    // 获取文件最后修改时间
    std::string GetFileLastWriteTime(const DirNameType& dirName);
    // 删除文件
    bool RemoveFile(const DirNameType& dirName);
    // 拷贝文件
    bool CopyFile(const DirNameType& oldFileName, const DirNameType& newFileName);
    // 重命名文件
    bool RenameFile(const DirNameType& oldFileName, const DirNameType& newFileName);
    
    // // 创建目录
    // bool CreateDirectory(const std::string& dirName);
    // // 重命名目录
    // bool RenameDirectory(const std::string& oldDirName, const std::string& newDirName);
    // // 拷贝目录
    // bool CopyDirectory(const std::string& oldDirName, const std::string& newDirName);
    // // 删除目录
    // bool RemoveDirectory(const std::string& dirName);
    // // 检测目录是否存在
    // bool IsDirectoryExist(const std::string& dirName);

    // /* 文件相关操作 */
    // // 创建文件
    // bool CreateFile(const std::string& fileName);
    // // 重命名文件
    // bool RenameFile(const std::string& oldFileName, const std::string& newFileName);
    // // 拷贝文件
    // bool CopyFile(const std::string& oldFileName, const std::string& newFileName);
    // // 删除文件
    // bool RemoveFile(const std::string& fileName);
    // // 查找文件是否存在
    // bool IsFileExist(const std::string& fileName);
    // // 查询文件大小
    // std::size_t GetFileSize(const std::string& fileName);
    // // 获取文件系统剩余容量
    // std::size_t GetFreeSpace(const std::string& dirName);
    // // 读取文件最后修改时间
    // std::string GetLastWriteTime(const std::string& fileName);
    // // 读取文件权限
    // std::string GetFilePermission(const std::string& fileName);
};