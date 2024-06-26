#pragma once

#include <string>
#include <fstream>

#include "boost/property_tree/ptree.hpp"

#include "logger.hpp"

namespace utility
{
    class ConfigParser
    {

    public:
        // 构造和析构
        ConfigParser();
        ~ConfigParser();

    public:
        // 读取配置文件
        bool Open(const std::string& filePath);
        // 读取配置文件内容
        bool Read(std::string& content);
        // 关闭配置文件流
        void Close();
        // 导入配置
        [[nodiscard]] virtual bool Load(const std::string& filePath) = 0;

    private:
        // 是否打开文件
        bool IsOpen();

    private:
        std::ifstream ifs_;
    };

    /**
     * @description: JSON配置文件解析器
     */
    class JsonConfigParser : private ConfigParser
    {
    public:
        // 导入配置
        [[nodiscard]] virtual bool Load(const std::string& filePath) override;

        // 获取值
        template<typename ValueType = std::string>
        [[nodiscard]] bool GetValue(const std::string& key, ValueType& value)
        {
            try
            {
                value = pt_.get<ValueType>(key);
            }
            catch (const std::exception& e)
            {
                log_error("JSON Parser can't parser key:{}", key);
                return false;
            }

            return true;
        }

    private:
        boost::property_tree::ptree pt_;
    };

    /**
     * @description: INI配置文件解析器
     */
    class IniConfigParser : private ConfigParser
    {
    public:
        // 导入配置
        [[nodiscard]] virtual bool Load(const std::string& filePath) override;

        // 获取值
        template<typename ValueType = std::string>
        [[nodiscard]] bool GetValue(const std::string& section, const std::string& key, ValueType& value)
        {
            try
            {
                value = pt_.get<ValueType>(section + "." + key);
            }
            catch (const std::exception& e)
            {
                log_error("INI Parser can't parser section:{}, key:{}", section, key);
                return false;
            }

            return true;
        }

    private:
        boost::property_tree::ptree pt_;
    };

};
