#pragma once
#include <iostream>
#include <fstream>
#include <thread>
#include <string>
#include <memory>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include "safequeue.h"
#include "logger.h"

using namespace utility;

namespace driver
{
    /**
     * @description: 文件传输类，外部接口使用transfer(file_full_path="/root/test.txt")传输文件
     *                                 使用receive(file_full_path="/root/recv_{FILENAME}_2023_12_31{.EXTENT}")来接收文件
     */
    class FileTransfer
    {
        public:
            using ftret_type = std::pair<bool, std::size_t>;
            using str_type   = std::string;
            using cstr_type  = const str_type;

            virtual ftret_type transfer(cstr_type file_full_path, cstr_type file_path, cstr_type file_name) = 0;
            virtual ftret_type receive(str_type file_full_path) = 0;
            virtual ~FileTransfer() = default;
    
            ftret_type transfer(cstr_type &file_full_path)
            {
                if (!checkFilePathExisted(file_full_path)) {
                    log_error("Don't existed file {}", file_full_path);
                    return ftret_type(false, 0);
                }
                auto ret = parseFileFullPath(file_full_path);
                return transfer(file_full_path, ret.first, ret.second);
            }
            ftret_type receive(cstr_type &file_path, cstr_type &extent_prefix, cstr_type &extent_suffix)
            {
                if (!checkFilePathExisted(file_path)) {
                    log_error("Don't existed path {}", file_path);
                    return ftret_type(false, 0);
                }
                str_type file_full_path = jointFilePathAndFileName(file_path, REP_FILENAME_SYMBOL + extent_prefix + extent_suffix);
                return receive(file_full_path);
            }
        private:
            // 检测文件路径是否存在
            inline bool checkFilePathExisted(cstr_type &file_full_path)
            {
                return boost::filesystem::exists(file_full_path);
            }
            // 解析文件完整路径，返回目录和文件名
            inline std::pair<str_type, str_type> parseFileFullPath(cstr_type &file_full_path)
            {
                boost::filesystem::path fp(file_full_path);
                str_type path = fp.parent_path().string();
                str_type name = fp.filename().string();
                return std::pair<str_type, str_type>(path, name);
            }
            // 拼接目录和文件名，返回文件完整路径
            inline str_type jointFilePathAndFileName(cstr_type &file_path, cstr_type &file_name)
            {
                auto file_fullpath = boost::filesystem::path(file_path) / file_name;
                return file_fullpath.string();
            }
        public:
            cstr_type REP_FILENAME_SYMBOL = "{FILENAME}";
    };

    /**
     * @description: 基于TCP信道的文件传输，文件过大需要基于TCP_TRANS_TUNK_SIZE进行分包，由TCP保证无需重传
     *               文件传输协议： target                                      server
     *                              --              connect                     -->
     *                             <--               "ok!"                      --
     *                              -- "${file_name}-${file_size}:${file_data}" -->
     */
    class TcpFileTransfer : public FileTransfer
    {
        using ioc_type      = boost::asio::io_context;
        using sock_type     = boost::asio::ip::tcp::socket;
        using accpetor_type = boost::asio::ip::tcp::acceptor;
        using port_type     = unsigned short;
        using callback_type = std::function<void()>;

        public:
            // 构造
            TcpFileTransfer(port_type server_port, const callback_type &server_readable_cb, cstr_type &target_ip, port_type target_port);
            // 析构
            ~TcpFileTransfer();
        public:
            // 发送文件
            ftret_type transfer(cstr_type file_full_path, cstr_type file_path, cstr_type file_name);
            // 接收文件
            ftret_type receive(str_type file_full_path);
        private:
            // 公共
            const std::size_t                       TCP_TRANS_TUNK_SIZE = 50;
            const std::string                       TCP_TRANS_SYNC_SYMBOL = "ok!";
            std::unique_ptr<ioc_type>               p_ioc;
            // 服务器相关
            port_type                               m_server_port;
            std::unique_ptr<sock_type>              p_server_socket;     
            std::unique_ptr<accpetor_type>          p_server_acceptor;
            SafeQueue<sock_type>                    m_client_request;
            callback_type                           f_readable_cb;
            // 客户端相关
            std::string                             m_target_ip;
            port_type                               m_target_port;
    };

    /**
     * @description: 基于串口信道的文件传输
     */
    class SerialFileTransfer : public FileTransfer
    {
        using ioc_type      = boost::asio::io_context;
        using serial_type   = boost::asio::serial_port;

        public:
            // 构造
            SerialFileTransfer(std::string port_name);
            // 析构
            ~SerialFileTransfer();
        public:
            // 发送文件
            ftret_type transfer(cstr_type file_full_path, cstr_type file_path, cstr_type file_name);
            // 接收文件
            ftret_type receive(str_type file_full_path);
        private:
            std::string                             m_portname;
            const int                               SERIAL_BAUDRATE = 115200;
            std::unique_ptr<ioc_type>               p_ioc;
            std::unique_ptr<serial_type>            p_serialport;
    };
}
