#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include "logger.h"
#include "tcpchannel.h"

using namespace utility;
using namespace driver;

TcpReceiver::TcpReceiver(unsigned short port)
    : m_port(port),
      m_ioc(),
      m_socket(m_ioc),
      m_acceptor(m_ioc, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), m_port))
{
    do_accpet();
}

void TcpReceiver::do_accpet()
{
    m_acceptor.async_accept(
        m_socket,
        [this] (boost::system::error_code ec)
        {
            log_critical("异步accept");
            if (!ec)
            {
                log_critical("没有错误");
                auto th = std::thread(
                    [this] ()
                    {
                        do_recv();
                    }
                );
                th.detach();
            }

            do_accpet();
        }
    );

}

void TcpReceiver::do_recv()
{
    log_info("接收到客户端连接");
}

TcpReceiver::~TcpReceiver()
{
    if (m_socket.is_open())
    {
        m_socket.close();
    }
}

TcpTransfer::TcpTransfer(const std::string &ip_addr, unsigned short port)
    : m_serverip(ip_addr),
      m_port(port),
      m_ioc(),
      m_socket(m_ioc)
{

}

bool TcpTransfer::transfer(const std::string &file_path, const std::string &file_name)
{
    std::string file = file_path + file_name;
    
    try
    {
        /* 检测文件是否存在 */
        std::ifstream ifs(file, std::ios::binary | std::ios::ate);
        if(!ifs.is_open())
        {
            log_error("Can't open file:{} when using tcp transfer {}:{}", file, m_serverip, m_port);
            return false;
        }

        /* 获取文件大小并将指针指向开始位置 */
        auto file_size = ifs.tellg();
        ifs.seekg(0);

        /* 建立tcp连接 */
        boost::system::error_code ec;
        boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address::from_string(m_serverip), m_port);
        m_socket.connect(ep, ec);
        if(ec)
        {
            log_error("Can't connect to tcp server({}:{}), when transfer file({}), error info:{}", m_serverip, m_port, file, boost::system::system_error(ec).what());
            return false;
        }

        /* 发送文件名+文件大小+文件内容 */
        boost::asio::write(m_socket, boost::asio::buffer(file_name + "\n"));
        boost::asio::write(m_socket, boost::asio::buffer(reinterpret_cast<const char *>(&file_size), sizeof(file_size)));
        std::vector<char> buf(4096);
        while (!ifs.eof())        // 分包发送
        {
            ifs.read(buf.data(), buf.size());
            boost::asio::write(m_socket, boost::asio::buffer(buf.data(), ifs.gcount()));
            if (ifs.peek() == EOF)
            {
                break;
            }
        }
        log_info("Successfully transfer file({}:{}) by tcp transfer({}:{})", file, file_size, m_serverip, m_port);
    }
    catch (const std::exception &e)
    {
        log_error("Failed to transmit file:{} to {}:{} by tcp transfer, error msg = {}", file, m_serverip, m_port, e.what());
        return false;
    }

    return true;
}

TcpTransfer::~TcpTransfer()
{
    if (m_socket.is_open())
    {
        m_socket.close();
    }
}