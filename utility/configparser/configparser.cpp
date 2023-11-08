#include <iostream>
#include <sstream>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/json_parser.hpp>


#include "configparser.h"

using namespace utility;
using namespace boost::property_tree;

/**********************************************************************************
 *************************    ConfigParser    *************************************
 **********************************************************************************/
bool ConfigParser::open(const std::string &filepath)
{
    /* 检测文件是否存在 */
    ifs.open(filepath);
    if(!is_open())
    {
        log_error("Can't open json file {}", filepath);
        return false;
    }

    return true;
}

bool ConfigParser::read(std::string &content)
{
    /* 打开文件 */
    if(!is_open())
    {
        log_error("Can't read ConfigFile's data");
        return false;
    }

    /* 读取文件内容 */
    std::stringstream buf;
    buf << ifs.rdbuf();
    content = buf.str();

    return true;
}

void ConfigParser::close()
{
    if(is_open())
    {
        /* 关闭文件 */
        ifs.close();
    }
}

bool ConfigParser::is_open()
{
    return ifs.is_open();
}

/**********************************************************************************
 *************************    JsonConfigParser    *********************************
 **********************************************************************************/
bool JsonConfigParser::load(const std::string &filepath)
{
    /* 打开文件 */
    if(!open(filepath))
    {
        log_error("Can't load json file {}", filepath);
        return false;
    }

    /* 解析json文件 */
    try
    {
        read_json(filepath, pt);
    }
    catch(const std::exception& e)
    {
        log_error("Can't parser json file {}", filepath);
        close();
        return false;
    }

    /* 关闭文件 */
    close();

    return true;

}

/**********************************************************************************
 *************************    IniConfigParser    **********************************
 **********************************************************************************/
bool IniConfigParser::load(const std::string &filepath)
{
    /* 打开文件 */
    if(!open(filepath))
    {
        log_error("Can't load ini file {}", filepath);
        return false;
    }

    /* 解析ini文件 */
    try
    {
        read_ini(filepath, pt);
    }
    catch(const std::exception& e)
    {
        log_error("Can't parser ini file {}", filepath);
        close();
        return false;
    }

    /* 关闭文件 */
    close();

    return true;
}