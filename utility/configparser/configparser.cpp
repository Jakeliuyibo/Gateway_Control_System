#include <iostream>
#include <fstream>
#include <sstream>

#include "configparser.h"

using namespace utility;

/**********************************************************************************
 *************************    ConfigParser    *************************************
 **********************************************************************************/
bool ConfigParser::open(const std::string &filepath, std::string &info)
{
    /* 检测文件是否存在 */
    ifs.open(filepath);
    if(!is_open())
    {
        info += "Can't open json file " + filepath;
        return false;
    }

    return true;
}

bool ConfigParser::read(std::string &content, std::string &info)
{
    /* 打开文件 */
    if(!is_open())
    {
        info += "Can't read file data";
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
bool JsonConfigParser::load(const std::string &filepath, std::string &info)
{
    /* 打开文件 */
    if(!open(filepath, info))
    {
        info += "Can't load json file";
        return false;
    }

    /* 读取文件内容 */
    std::string content;
    if(!read(content, info))
    {
        info += "Can't parser json file";
        close();
        return false;
    }

    /* Json解析文件 */
    Json::Reader reader;
    reader.parse(content, root);

    /* 关闭文件 */
    close();

    return true;
}


/**********************************************************************************
 *************************    IniConfigParser    **********************************
 **********************************************************************************/
bool IniConfigParser::load(const std::string &filepath,std::string &info)
{
    /* 打开文件 */
    if(!open(filepath, info))
    {
        info += "Can't load ini file";
        return false;
    }

    /* 解析ini文件 */
    try
    {
        boost::property_tree::read_ini(filepath, pt);
    }
    catch(const std::exception& e)
    {
        info += "Can't parser ini file";
        close();
        return false;
    }

    /* 关闭文件 */
    close();

    return true;
}