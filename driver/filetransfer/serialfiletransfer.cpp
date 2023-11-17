#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include "filetransfer.h"

using namespace utility;
using namespace driver;



SerialFileTransfer::SerialFileTransfer(std::string port_name)
    :
    m_portname(port_name),
    p_ioc(std::make_unique<ioc_type>()),
    p_serialport(std::make_unique<serial_type>(*p_ioc))
{
    boost::system::error_code ec;

    /* open Serial port object */
    p_serialport->open(m_portname, ec);
    if (ec)
    {
        log_error("打开串口({})失败, msg = {}", m_portname, ec.message());
        return;
    }
	
	/* set port argument       */
	p_serialport->set_option(boost::asio::serial_port::baud_rate(SERIAL_BAUDRATE), ec);
	p_serialport->set_option(boost::asio::serial_port::flow_control(boost::asio::serial_port::flow_control::none), ec);
	p_serialport->set_option(boost::asio::serial_port::parity(boost::asio::serial_port::parity::none), ec);
	p_serialport->set_option(boost::asio::serial_port::stop_bits(boost::asio::serial_port::stop_bits::one), ec);
	p_serialport->set_option(boost::asio::serial_port::character_size(8), ec);
    if (ec)
    {
        log_error("设置串口({})参数异常, msg = {}", m_portname, ec.message());
        return;
    }
}

SerialFileTransfer::~SerialFileTransfer()
{
    if (p_serialport->is_open())
    {
        p_serialport->close();
    }
}

SerialFileTransfer::ftret_type SerialFileTransfer::transfer(cstr_type file_full_path, cstr_type file_path, cstr_type file_name)
{
    std::size_t trans_bytes = 0;
    bool ret = true;
    
    // try
    // {
    //     /* 打开文件并获取文件大小 */
    //     std::ifstream ifs(file_full_path, std::ios::binary | std::ios::ate);
    //     auto file_size = ifs.tellg();
    //     ifs.seekg(0, std::ios::beg);

    //     /* 读取文件内容 */
    //     std::vector<char> buf(file_size);
    //     ifs.read(buf.data(), file_size);
    //     std::string msg = file_name + "-" + std::to_string(file_size) + "\n" + std::string(buf.begin(), buf.end());

    //     /* 串口发送数据 */
    //     trans_bytes += boost::asio::write(*p_serialport, boost::asio::buffer(msg));

    //     /* 释放资源 */
    //     ifs.close();
    //     log_info("SerialFileTransfer({})成功传输文件({}:{} bytes)", m_portname, file_full_path, file_size);
    // }
    // catch(const std::exception& e)
    // {
    //     log_error("SerialFileTransfer({})传输文件file({})失败, error msg = {}", m_portname, file_full_path, e.what());
    //     ret = false;
    // }
    
    return std::pair<bool, size_t>(ret, trans_bytes);
}

SerialFileTransfer::ftret_type SerialFileTransfer::receive(str_type file_full_path)
{
    std::size_t recv_bytes = 0;
    bool ret = true;

    // /* 检测路径是否存在 */
    // if (!checkFilePathExisted(file_path))
    // {
    //     log_error("Don't existed file path {}", file_path);
    //     return std::pair<bool, size_t>(false, 0);
    // }

    // /* 接收文件 */
    // try
    // {
    //     // 接收头部信息
    //     boost::asio::streambuf header_streambuf;
    //     recv_bytes += boost::asio::read_until(*p_serialport, header_streambuf, '\n');
    //     std::istream header_istream(&header_streambuf);
    //     std::string header;
    //     std::getline(header_istream, header);
    //     std::size_t separator_pos = header.find('-');
    //     if (separator_pos == std::string::npos)
    //     {
    //         throw std::runtime_error("Not find separator.");
    //     }
    //     std::string file_name = header.substr(0, separator_pos);
    //     std::size_t file_size = std::stoull(header.substr(separator_pos + 1));

    //     // 接收文件内容并写入存储路径下文件
    //     // 打开写入文件
    //     std::string file_complete_path = file_path + prefix + file_name + suffix;
    //     std::ofstream ofs(file_complete_path, std::ios::binary | std::ios::trunc);
    //     if (!ofs.is_open())
    //     {
    //         throw std::runtime_error("File not existed.");
    //     }
    //     // 读取文件内容
    //     std::vector<char> buf(file_size);
    //     recv_bytes += boost::asio::read(*p_serialport, boost::asio::buffer(buf.data(), file_size));
    //     ofs.write(buf.data(), recv_bytes);

    //     ofs.flush();
    //     ofs.close();

    //     log_info("SerialFileTransfer({})成功接收到客户端文件({}:{} bytes)", m_portname, file_name, file_size);
    // }
    // catch(const std::exception& e)
    // {
    //     log_error("SerialFileTransfer({})接收文件失败, error msg = {}", m_portname, e.what());
    //     ret = false;
    // }
    
    return std::pair<bool, size_t>(ret, recv_bytes);
}
