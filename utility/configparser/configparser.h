#pragma once

#include <string>
#include <fstream>
#include <boost/property_tree/ptree.hpp>  
#include "logger.h"

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
            bool open(const std::string &filepath);
            // 读取配置文件内容
            bool read(std::string &content);
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
            bool load(const std::string &filepath);

            template<typename T>
            bool getValue(const std::string &key,T &value)
            {
                try
                {
                    value = pt.get<T>(key);
                }
                catch(const std::exception& e)
                {
                    error("JSON Parser can't parser key:{}" + key);
                    return false;
                }

                return true;
            }
        private:
            boost::property_tree::ptree pt;
    };

    /**
     * @description: INI配置文件解析器
     */
    class IniConfigParser : public ConfigParser
    {
        public:
            bool load(const std::string &filepath);

            template<typename T>
            bool getValue(const std::string &section, const std::string &key, T &value)
            {
                try
                {
                    value = pt.get<T>(section + "." + key);
                }
                catch(const std::exception& e)
                {
                    error("INI Parser can't parser section:{},key:{}", section, key);
                    return false;
                }

                return true;
            }
        private:
            boost::property_tree::ptree pt;
    };

};
