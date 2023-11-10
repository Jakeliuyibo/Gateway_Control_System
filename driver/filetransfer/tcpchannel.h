#pragma once

#include <string>
#include <memory>
#include <boost/asio.hpp>

namespace driver
{
    using tcpsocket_type = boost::asio::ip::tcp::socket;

    class Session
    {
        public:
            Session(tcpsocket_type sock) {}
            ~Session() {}
            void start() {}
    };

    class TcpServer
    {
        public:
            // 构造
            TcpServer(const std::string &ip_addr, short port=1234);
            // 析构
            ~TcpServer();
        private:
            void do_accept();
        private:
            short                           m_port;
            boost::asio::io_context         m_ioc;
            tcpsocket_type                  m_socket;
            boost::asio::ip::tcp::acceptor  m_acceptor;
    };
}