#include "logger.h"
#include "tcpchannel.h"

using namespace utility;
using namespace driver;

TcpServer::TcpServer(const std::string &ip_addr, short port)
    : m_port(port),
      m_ioc(),
      m_socket(m_ioc),
      m_acceptor(m_ioc, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), m_port))
{
    do_accept();
}

TcpServer::~TcpServer()
{

}

void TcpServer::do_accept()
{
    m_acceptor.async_accept(m_socket,
        [this](boost::system::error_code ec)
        {
            if(!ec)
            {
                std::make_shared<Session>(std::move(m_socket))->start();
            }

            do_accept();
        }
    );
}
