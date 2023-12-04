#pragma once
#include <iostream>
#include <fstream>
#include <thread>
#include <string>
#include <memory>
#include <chrono>
#include <mutex>
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
        using callback_type = std::function<void()>;

        public:
            enum PROTOCOL_CMD
            {
                CMD_FILEINFO = 0x01,     // 文件信息
                CMD_FILEID   = 0x02,     // 文件ID
                CMD_TUNKDATA = 0x03,     // 块数据
                CMD_ACK      = 0x04,     // ACK信息
                CMD_RETRANS  = 0x05      // 重传块数据
            };
            struct FileDescription
            {
                std::size_t file_id;
                std::string file_name;
                std::size_t file_size;
                std::size_t tunk_size;
                std::map<std::size_t, std::string> tunk_data;
                FileDescription() {}
                FileDescription(std::size_t _id, std::string _name, std::size_t _filesize, std::size_t _tunksize)
                    : file_id(_id), file_name(_name), file_size(_filesize), tunk_size(_tunksize) {}
            };
            struct ProtocolPackage
            {
                PROTOCOL_CMD cmd;
                std::vector<uint8_t> payload;
                ProtocolPackage() {}
                ProtocolPackage(PROTOCOL_CMD c, std::vector<uint8_t> p) : cmd(c), payload(p) {}
            };

        public:
            // 构造
            SerialFileTransfer(std::string port_name, const callback_type &server_readable_cb);
            // 析构
            ~SerialFileTransfer();
        public:
            // 发送文件
            ftret_type transfer(cstr_type file_full_path, cstr_type file_path, cstr_type file_name);
            // 接收文件
            ftret_type receive(str_type file_full_path);
        private:
            // 创建子线程监听客户端数据
            void _subthread_listen_client();
            // 创建子线程监听服务器文件管理
            void _subthread_listen_manage_server_filedescription();
        private:
            // 转化协议帧为十六进制字符串
            std::string _convert_vecu8_to_hexstring(std::vector<uint8_t> &frame);
            // 校验协议帧
            void _check_protocol_frame(std::vector<uint8_t> &frame, PROTOCOL_CMD &cmd);
            // 解析文件信息
            void _parse_fileinfo_from_payload(std::vector<uint8_t> &payload, std::string &file_name, std::size_t &file_size, std::size_t &tunk_size);
            // 解析文件ID
            void _parse_fileid_from_payload(std::vector<uint8_t> &payload, std::string &file_name, std::size_t &file_id);
            // 解析块数据
            void _parse_tunkdata_from_payload(std::vector<uint8_t> &payload, std::size_t &file_id, std::size_t &tunk_id, std::string &tunk_data);
            // 解析ACK数据
            void _parse_ack_from_payload(std::vector<uint8_t> &payload, std::size_t &file_id);
            // 解析重传数据
            void _parse_retrans_from_payload(std::vector<uint8_t> &payload, std::size_t &file_id, std::vector<std::size_t> &retrans_tunk_id);
            // 通过'-'\':'\'\x00'等符号分隔payload
            void _separate_payload_by_symbol(std::vector<uint8_t> &payload, std::vector<std::string> &parse, int &cnt_dashes, int &cnt_semicolons);
            // 写入一帧数据，payload长度不能超过PROTOCOL_PAYLOAD_LEN
            std::size_t _write_bytes(PROTOCOL_CMD cmd, std::vector<uint8_t> payload);
        private:
            // 公共
            const std::size_t                       PROTOCOL_LEN = 10;                  // 串口协议长度
            const std::vector<uint8_t>              PROTOCOL_START_BITS = {0x69, 0X96}; // 起始位
            const std::vector<uint8_t>              PROTOCOL_END_BITS = {0x0D, 0x0A};   // 停止位
            const std::size_t                       PROTOCOL_SB_LEN = PROTOCOL_START_BITS.size();
            const std::size_t                       PROTOCOL_CMD_LEN = 1;
            const std::size_t                       PROTOCOL_EB_LEN = PROTOCOL_END_BITS.size();
            const std::size_t                       PROTOCOL_PAYLOAD_LEN = PROTOCOL_LEN
                                                                            - PROTOCOL_SB_LEN
                                                                            - PROTOCOL_CMD_LEN
                                                                            - PROTOCOL_EB_LEN; 

            const int                               SERIAL_BAUDRATE = 115200;       // 波特率
            const int                               SERIAL_MAXIMUM_RETRANS = 3;     // 最大重传次数

            std::unique_ptr<ioc_type>               p_ioc;
            std::string                             m_portname;
            std::unique_ptr<serial_type>            p_serialport;
            // 服务器相关：接收文件
            SafeQueue<ProtocolPackage>              s_rbuf;                         // 服务端接收到的协议包缓存
            callback_type                           f_readable_cb;                  // 监听线程通知上层调用receive处理协议包的回调
            const std::vector<std::size_t>          FILEID_RANGE = {0, 10};         // 服务端文件ID范围
            SafeQueue<std::size_t>                  s_fileid_allocator;             // 服务端文件ID分配器
            std::mutex                              s_file_management_lock;
            std::map<std::size_t, FileDescription>  s_file_management;              // 服务器接收到的文件管理
            // 客户端相关：发送文件
            std::map<std::size_t, FileDescription>  c_file_management;              // 客户端发送的文件管理


    };
}
