#include "configparser.hpp"

#include <iostream>
#include <sstream>

#include "boost/property_tree/ini_parser.hpp"
#include "boost/property_tree/json_parser.hpp"

using namespace utility;

/**********************************************************************************
 *************************    ConfigParser    *************************************
 **********************************************************************************/
ConfigParser::ConfigParser()
{

}

ConfigParser::~ConfigParser()
{
    Close();
}

bool ConfigParser::Open(const std::string& filePath)
{
    /* 检测文件是否存在 */
    ifs_.open(filePath);
    if (!IsOpen())
    {
        log_error("Can't open json file ");
        log_error("Can't open json file {}", filePath);
        return false;
    }

    return true;
}

bool ConfigParser::Read(std::string& content)
{
    /* 打开文件 */
    if (!IsOpen())
    {
        log_error("Can't read ConfigFile's data");
        return false;
    }

    /* 读取文件内容 */
    std::stringstream buf;
    buf << ifs_.rdbuf();
    content = buf.str();

    return true;
}

void ConfigParser::Close()
{
    if (IsOpen())
    {
        /* 关闭文件 */
        ifs_.close();
    }
}

bool ConfigParser::IsOpen()
{
    return ifs_.is_open();
}

/**********************************************************************************
 *************************    JsonConfigParser    *********************************
 **********************************************************************************/
bool JsonConfigParser::Load(const std::string& filePath)
{
    /* 打开文件 */
    if (!Open(filePath))
    {
        log_error("Can't load json file {}", filePath);
        return false;
    }

    /* 解析json文件 */
    try
    {
        boost::property_tree::read_json(filePath, pt_);
    }
    catch (const std::exception& e)
    {
        log_error("Can't parser json file {}", filePath);
        Close();
        return false;
    }

    /* 关闭文件 */
    Close();

    return true;
}

/**********************************************************************************
 *************************    IniConfigParser    **********************************
 **********************************************************************************/
bool IniConfigParser::Load(const std::string& filePath)
{
    /* 打开文件 */
    if (!Open(filePath))
    {
        log_error("Can't load ini file {}", filePath);
        return false;
    }

    /* 解析ini文件 */
    try
    {
        boost::property_tree::read_ini(filePath, pt_);
    }
    catch (const std::exception& e)
    {
        log_error("Can't parser ini file {}", filePath);
        Close();
        return false;
    }

    /* 关闭文件 */
    Close();

    return true;
}