#include "filetransfer.h"

using namespace utility;
using namespace driver;

TcpFileTransfer::TcpFileTransfer(port_type server_port, const callback_type &server_readable_cb, cstr_type &target_ip, port_type target_port)
    :
    m_server_port(server_port),
    f_readable_cb(server_readable_cb),
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
                    log_info("TcpFileTransfer({})接收到客户端({})连接", m_server_port, remote_ip);

                    /* 回调通知上层模块数据可读 */
                    f_readable_cb();
                }
            }
            catch (const std::exception &e)
            {
                log_error("TcpFileTransfer({})监听客户端连接出错，msg = {}", m_server_port, e.what());
            }
        }
    ).detach();
}

TcpFileTransfer::~TcpFileTransfer()
{
    p_server_acceptor->close();
    p_server_socket->close();
}

TcpFileTransfer::ftret_type TcpFileTransfer::transfer(cstr_type file_full_path, cstr_type file_path, cstr_type file_name)
{
    std::size_t trans_bytes = 0;
    bool ret = true;

    try
    {
        /* 打开文件并获取文件大小 */
        std::ifstream ifs(file_full_path, std::ios::binary | std::ios::ate);
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
        std::string header = file_name + "-" + std::to_string(file_size) + ":";
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

        /* 释放资源 */
        ifs.close();
        sk.close();
        log_info("TcpFileTransfer成功传输文件({}:{} bytes) to ({}:{})", file_full_path, file_size, m_target_ip, m_target_port);
    }
    catch(const std::exception& e)
    {
        log_error("TcpFileTransfer向({}:{})传输文件file({})失败, error msg = {}", m_target_ip, m_target_port, file_full_path, e.what());
        ret = false;
    }

    return ftret_type(ret, trans_bytes);
}

TcpFileTransfer::ftret_type TcpFileTransfer::receive(str_type file_full_path)
{
    std::size_t recv_bytes = 0;
    bool ret = true;

    /* 获取客户端 */
    sock_type client_sock(*p_ioc);
    if (!m_client_request.dequeue(client_sock))
    {
        log_error("TcpFileTransfer({})获取客户端请求失败", m_server_port);
        return ftret_type(false, 0);
    }

    /* 接收文件 */
    try
    {
        std::vector<char> buf(1, '0');

        /* 接收头部信息 */
        // 读取文件名和文件大小
        std::string file_name = "";
        std::size_t file_size = 0;
        {
            std::string header = "";
            while (1) {
                recv_bytes += boost::asio::read(client_sock, boost::asio::buffer(buf.data(), 1));
                if (buf[0] == '-') {
                    file_name = header;
                    header = "";
                } else if (buf[0] == ':') {
                    file_size = std::stoull(header);
                    break;
                } else {
                    header += buf[0];
                }
            }
        }

        // 替换文件完整路径
        std::size_t rep_filename_pos = file_full_path.find(REP_FILENAME_SYMBOL);
        if (rep_filename_pos == std::string::npos)
        {
            throw std::runtime_error("Can't find replace FILENAME symbol.");
        }
        file_full_path.replace(rep_filename_pos, REP_FILENAME_SYMBOL.size(), file_name);

        /* 接收文件内容并写入存储路径下文件 */
        // 打开写入文件
        std::ofstream ofs(file_full_path, std::ios::binary | std::ios::trunc);
        if (!ofs.is_open())
        {
            throw std::runtime_error("File not existed.");
        }
        // 读取文件内容
        std::size_t remaining_bytes = file_size;
        buf.clear();
        buf.resize(std::min(remaining_bytes, TCP_TRANS_TUNK_SIZE));
        while (remaining_bytes > 0)
        {
            std::size_t bytes_to_read = std::min(remaining_bytes, TCP_TRANS_TUNK_SIZE);
            recv_bytes += boost::asio::read(client_sock, boost::asio::buffer(buf.data(), bytes_to_read));
            ofs.write(buf.data(), bytes_to_read);
            remaining_bytes -= bytes_to_read;
        }
        ofs.flush();
        ofs.close();

        log_info("TcpFileTransfer({})成功接收到客户端文件({}:{} bytes)", m_server_port, file_full_path, file_size);
    }
    catch(const std::exception& e)
    {
        log_error("TcpFileTransfer({})接收文件({})失败, error msg = {}", m_server_port, file_full_path, e.what());
        ret = false;
    }

    /* 释放资源 */
    client_sock.close();
    return ftret_type(ret, recv_bytes);
}
