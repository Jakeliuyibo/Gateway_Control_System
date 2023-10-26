#pragma once

#include <string>
#include <jsoncpp/json/json.h>
#include <boost/property_tree/ptree.hpp>  
#include <boost/property_tree/ini_parser.hpp>

namespace utility
{
    class ConfigParser
    {
        public:
            ConfigParser() {}
            ~ConfigParser() 
            {
                close();
            }
            // 读取配置文件
            bool open(const std::string &filepath, std::string &info);
            // 读取配置文件内容
            bool read(std::string &content, std::string &info);
            // 关闭配置文件流
            void close();
        private:
            bool is_open();
        private:
            std::ifstream ifs;
    };

    /**
     * @description: JSON配置文件解析器
     */
    class JsonConfigParser : public ConfigParser
    {
        public:
            bool load(const std::string &filepath, std::string &info);

            template<typename T>
            bool getValue(const std::string &key,T &value, std::string &info)
            {
                /* 检测一级成员是否存在 */
                if(!root.isMember(key))
                {
                    info += "JSON data is not find key " + key;
                    return false;
                }

                try
                {
                    Json::Value json_value = root[key];
                    value = json_value.as<T>();
                    return true;
                }
                catch(const std::exception& e)
                {
                    info += "JSON Parser can't parser key " + key;
                    return false;
                }
            }
        private:
            Json::Value root;
    };

    class IniConfigParser : public ConfigParser
    {
        public:
            bool load(const std::string &filepath,std::string &info);

            template<typename T>
            bool getValue(const std::string &section, const std::string &key, T &value, std::string &info)
            {
                try
                {
                    value = pt.get<T>(section + "." + key);
                }
                catch(const std::exception& e)
                {
                    info += "INI Parser can't parser section " + section + ",key " + key;
                    return false;
                }

                return true;
            }
        private:
            boost::property_tree::ptree pt;
    };

};
