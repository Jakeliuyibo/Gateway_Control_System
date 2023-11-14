#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include "transchannel.h"

using namespace utility;
using namespace driver;

/**********************************************************************************
 ******************************    TCP 文件传输    ********************************
 **********************************************************************************/

TcpFTChannel::TcpFTChannel(port_type server_port, const std::string &target_ip, port_type target_port)
    :
    m_server_port(server_port),
    m_target_ip(target_ip),
    m_target_port(target_port),
    p_ioc(std::make_unique<ioc_type>()),
    p_server_socket(std::make_unique<sock_type>(*p_ioc)),
    p_server_acceptor(std::make_unique<accpetor_type>(*p_ioc, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), m_server_port)))
{
    /* 设置监听复用 */
    p_server_acceptor->set_option(boost::asio::socket_base::reuse_address(true));

    /* 创建子线程监听客户端连接 */
    std::thread(
        [this] ()
        {
            try
            {
                for(;;)
                {
                    /* 阻塞等待客户端连接 */
                    sock_type client_sk(*p_ioc);
                    p_server_acceptor->accept(client_sk);
                    auto remote_ip = client_sk.remote_endpoint().address().to_string();

                    /* 多线程处理 */
                    m_client_request.enqueue(std::move(client_sk));
                    log_info("TcpFTChannel({})接收到客户端({})连接", m_server_port, remote_ip);

                    // std::thread(&TcpFTChannel::recv, this, std::move(client_sk)).detach();
                }
            }
            catch (const std::exception &e)
            {
                log_error("TcpFTChannel({})监听客户端连接出错，msg = {}", m_server_port, e.what());
            }
        }
    ).detach();
}

TcpFTChannel::~TcpFTChannel()
{
    p_server_acceptor->close();
    p_server_socket->close();
}

std::pair<bool, size_t> TcpFTChannel::transfer(const std::string &file_path, const std::string &file_name)
{
    std::size_t trans_bytes = 0;
    bool ret = true;

    std::string file_complete_path = file_path + file_name;
    std::ifstream ifs(file_complete_path, std::ios::binary | std::ios::ate);
    /* 检测文件是否存在 */
    if (!ifs.is_open())
    {
        log_error("Don't existed file {}", file_complete_path);
        return std::pair<bool, size_t>(false, 0);
    }

    try
    {
        /* 获取文件大小并将指针指向开始位置 */
        auto file_size = ifs.tellg();
        ifs.seekg(0, std::ios::beg);

        /* 创建客户端连接至目标服务器 */
        sock_type sk(*p_ioc);
        boost::system::error_code ec;
        boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address::from_string(m_target_ip), m_target_port);
        sk.connect(ep, ec);
        if (ec)
        {
            throw std::runtime_error("Connect server failed.");
        }

        /* 发送文件名+文件大小 */
        std::string header = file_name + "-" + std::to_string(file_size) + "\n";
        trans_bytes += boost::asio::write(sk, boost::asio::buffer(header));

        /* 发送文件内容 */
        std::vector<char> buf(TCP_TRANS_TUNK_SIZE);
        std::size_t remaining_bytes = file_size;
        while (!ifs.eof() && remaining_bytes > 0)   // 分包发送
        {
            std::size_t bytes_to_write = std::min(remaining_bytes, TCP_TRANS_TUNK_SIZE);
            ifs.read(buf.data(), bytes_to_write);
            trans_bytes += boost::asio::write(sk, boost::asio::buffer(buf.data(), bytes_to_write));
            remaining_bytes -= bytes_to_write;
        }

        log_info("TcpFTChannel成功传输文件({}:{} bytes) to ({}:{})", file_complete_path, file_size, m_target_ip, m_target_port);
    }
    catch(const std::exception& e)
    {
        log_error("TcpFTChannel向({}:{})传输文件file({})失败, error msg = {}", m_target_ip, m_target_port, file_complete_path, e.what());
        ret = false;
    }

    /* 释放资源 */
    ifs.close();
    return std::pair<bool, size_t>(ret, trans_bytes);
}

std::pair<bool, size_t> TcpFTChannel::recvive(const std::string &file_path, const std::string &prefix, const std::string &suffix)
{
    std::size_t recv_bytes = 0;
    bool ret = true;

    /* 检测路径是否存在 */
    if (!boost::filesystem::exists(file_path))
    {
        log_error("Don't existed file path {}", file_path);
        return std::pair<bool, size_t>(false, 0);
    }

    /* 获取客户端 */
    sock_type client_sock(*p_ioc);
    if (!m_client_request.dequeue(client_sock))
    {
        log_error("TcpFTChannel({})获取客户端请求失败", m_server_port);
        return std::pair<bool, size_t>(false, 0);
    }

    /* 接收文件 */
    try
    {
        // 接收头部信息
        boost::asio::streambuf header_streambuf;
        recv_bytes += boost::asio::read_until(client_sock, header_streambuf, '\n');
        std::istream header_istream(&header_streambuf);
        std::string header;
        std::getline(header_istream, header);
        std::size_t separator_pos = header.find('-');
        if (separator_pos == std::string::npos)
        {
            throw std::runtime_error("Not find separator.");
        }
        std::string file_name = header.substr(0, separator_pos);
        std::size_t file_size = std::stoull(header.substr(separator_pos + 1));

        // 接收文件内容并写入存储路径下文件
        // 打开写入文件
        std::string file_complete_path = file_path + prefix + file_name + suffix;
        std::ofstream ofs(file_complete_path, std::ios::binary | std::ios::trunc);
        if (!ofs.is_open())
        {
            throw std::runtime_error("File not existed.");
        }
        // 读取文件内容
        std::vector<char> buf(TCP_TRANS_TUNK_SIZE);
        std::size_t remaining_bytes = file_size;
        while (remaining_bytes > 0)
        {
            std::size_t bytes_to_read = std::min(remaining_bytes, TCP_TRANS_TUNK_SIZE);
            recv_bytes += boost::asio::read(client_sock, boost::asio::buffer(buf.data(), bytes_to_read));
            ofs.write(buf.data(), bytes_to_read);
            remaining_bytes -= bytes_to_read;
        }
        ofs.flush();
        ofs.close();

        log_info("TcpFTChannel({})成功接收到客户端文件({}:{} bytes)", m_server_port, file_name, file_size);
    }
    catch(const std::exception& e)
    {
        log_error("TcpFTChannel({})接收文件失败, error msg = {}", m_server_port, e.what());
        ret = false;
    }

    /* 释放资源 */
    client_sock.close();
    return std::pair<bool, size_t>(ret, recv_bytes);
}

/**********************************************************************************
 ******************************    串口 发送端    **********************************
 **********************************************************************************/
// Serial::Serial(IniConfigParser *parser)
// {
//     bool parserFlag = true;

//     /* 解析参数 */
//     parserFlag &= parser->getValue<std::string>("SERIAL", "SERIAL_PORT"    , m_portname);
//     parserFlag &= parser->getValue<int        >("SERIAL", "SERIAL_BAUDRATE", m_baudrate);
//     if(!parserFlag)
//     {
//         log_error("Init serial failed, when load config");
//         return;
//     }

//     /* 初始化串口 */
//     p_serialport = new boost::asio::serial_port(m_ios);
//     if (p_serialport)
//     {
// 		if(!open())
//         {
//             log_error("Can't open serial {}", m_portname);
//             return;
//         }
// 	}
// }

// Serial::~Serial()
// {
//     close();
// }

// bool Serial::open()
// {
//     boost::system::error_code ec;

// 	if (!p_serialport)
//     {
// 		return false;
// 	}
 
//     /* open Serial port object */
//     p_serialport->open(m_portname, ec);
	
// 	/* set port argument       */
// 	p_serialport->set_option(serial_port::baud_rate(m_baudrate), ec);
// 	p_serialport->set_option(serial_port::flow_control(serial_port::flow_control::none), ec);
// 	p_serialport->set_option(serial_port::parity(serial_port::parity::none), ec);
// 	p_serialport->set_option(serial_port::stop_bits(serial_port::stop_bits::one), ec);
// 	p_serialport->set_option(serial_port::character_size(8), ec);
//     if(ec.value() != 0)
//     {
//         log_error("Open serial {} failed, error info = {}", m_portname, ec.message());
//         return false;
//     }

// 	return true;
// }


// void Serial::read()
// {
//     // boost::asio::read();
// }

// void Serial::write(const std::string &data)
// {
//     boost::system::error_code ec;

//     size_t len = boost::asio::write(*p_serialport, buffer(data), ec);
//     if(ec.value() != 0)
//     {
//         log_error("Write data to serial {} failed, error info = {}", m_portname, ec.message());
//         return;
//     }
// }

// void Serial::close()
// {
//     if(p_serialport)
//     {
//         p_serialport->close();
//         delete p_serialport;
//         p_serialport = nullptr;
//     }
// }