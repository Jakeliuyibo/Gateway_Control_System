#include "filemanagement.hpp"

#include <chrono>
#include <iomanip>

#include "logger.hpp"

namespace utility
{

// 文件是否存在
bool IsFileExisted(const DirNameType& dirName)
{
    DirType dir(dirName);
    return IsDirExisted(dirName) && std::filesystem::is_regular_file(dir);
}

// 解析文件名
FileNameType GetFileName(const DirNameType& dirName)
{
    DirType dir(dirName);
    return dir.filename();
}

// 解析文件扩展名
FileNameType GetFileExtensionName(const DirNameType& dirName)
{
    DirType dir(dirName);
    return dir.extension();
}

// 修改文件扩展名
bool ModifyFileExtensionName(DirNameType& dirName, const FileNameType& modifyExtensionName)
{
    DirType dir(dirName);
    if (!dir.has_extension())
    {
        log_warning("The file path {} doesn't exist extension", dirName);
        return false;
    }

    DirType modifyExtension(modifyExtensionName);
    dir.replace_extension(modifyExtension);
    dirName = dir.string();
    return true;
}

// 获取文件大小
FileSizeType GetFileSize(const DirNameType& dirName)
{
    if (!IsFileExisted(dirName))
    {   
        log_error("Don't exist file {} when calculating file size", dirName);
        return 0;
    }
    
    DirType dir(dirName);
    return std::filesystem::file_size(dir);
}

// 获取文件最后修改时间
std::string GetFileLastWriteTime(const DirNameType& dirName)
{
    if (!IsFileExisted(dirName))
    {   
        log_error("Don't exist file {} when getting file last write time", dirName);
        return "";
    }

    DirType dir(dirName);
    auto lastWriteTime = std::filesystem::last_write_time(dir);
	auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(lastWriteTime - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now());
	auto tt = std::chrono::system_clock::to_time_t(sctp);
 
	std::tm* gmt = std::localtime(&tt); // UTC: std::gmtime(&tt);
	std::stringstream buffer;
	buffer << std::put_time(gmt, "%Y-%m-%d %H:%M:%S");
	return buffer.str();
}

// 删除文件
bool RemoveFile(const DirNameType& dirName)
{
    DirType dir(dirName);
    return std::filesystem::remove(dir);
}










};