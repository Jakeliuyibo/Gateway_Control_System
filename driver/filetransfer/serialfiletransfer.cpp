#include <sstream>
#include "filetransfer.h"

using namespace utility;
using namespace driver;

SerialFileTransfer::SerialFileTransfer(std::string port_name, const callback_type &server_readable_cb)
    :
    m_portname(port_name),
    f_readable_cb(server_readable_cb),
    p_ioc(std::make_unique<ioc_type>()),
    p_serialport(std::make_unique<serial_type>(*p_ioc))
{
    boost::system::error_code ec;

    /* 打开串口 */
    p_serialport->open(m_portname, ec);
    if (ec)
    {
        log_error("打开串口({})失败, msg = {}", m_portname, ec.message());
        return;
    }
	
	/* 设置串口参数 */
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

    /* 构建客户端请求文件ID分配器 */
    for(int idx = FILEID_RANGE[0]; idx < FILEID_RANGE[1]; idx++)
    {
        s_fileid_allocator.enqueue(idx);
    }

    /* 创建子线程监听客户端连接 */
    std::thread(&SerialFileTransfer::_subthread_listen_client, this).detach();

    /* 创建子线程监听服务器文件管理 */
    std::thread(&SerialFileTransfer::_subthread_listen_manage_server_filedescription, this).detach();
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
    std::size_t recv_bytes = 0;             // 接收字节
    std::size_t trans_bytes = 0;            // 发送字节
    bool ret = true;                        // 发送成功

    std::ifstream ifs;                      // 文件流

    /* 创建文件转发描述符 */ 
    {
        std::unique_lock<std::mutex> lck(c_file_management_lock);
        c_file_management[file_name] = std::make_shared<FileTransferDescription>();
    }
    
    FileTransferDescription *p_fd = c_file_management[file_name].get();
    std::unique_lock<std::mutex> lck(p_fd->block_lock);

    /* 读取文件信息并计算相关信息 */
    try
    {
        ifs.open(file_full_path, std::ios::binary | std::ios::ate);
        p_fd->file_size = ifs.tellg();
        p_fd->tunk_size = static_cast<std::size_t>(std::ceil(static_cast<double>(p_fd->file_size) / PROTOCOL_ACTUAL_PAYLOAD_LEN));
        ifs.seekg(0, std::ios::beg);

        /* 读取文件加载到内存中 */
        {
            std::size_t remaining_bytes = p_fd->file_size;
            std::vector<char> tunk(PROTOCOL_ACTUAL_PAYLOAD_LEN);
            while (!ifs.eof() && remaining_bytes > 0)   // 分包发送
            {
                std::size_t bytes_to_write = std::min(remaining_bytes, PROTOCOL_ACTUAL_PAYLOAD_LEN);
                ifs.read(tunk.data(), bytes_to_write);
                std::string tunk_str(tunk.begin(), tunk.begin() + bytes_to_write);
                p_fd->tunk_data.emplace_back(tunk_str);
                remaining_bytes -= bytes_to_write;
            }
        }

        /* 释放资源 */
        ifs.close();
    }
    catch(const std::exception& e)
    {
        log_error("SerialFTP({})获取文件{})失败, error msg = {}", m_portname, file_full_path, e.what());
        goto err_ret;
    }

    /* 串口传输文件 */
    try
    {
        /* 1、传输文件信息：${cmd:1}${file_name}-${file_size}-${tunk_size} */
        trans_bytes += _write_bytes(CMD_FILEINFO, file_name
                                                + PROTOCOL_SEPARATOR
                                                + std::to_string(p_fd->file_size)
                                                + PROTOCOL_SEPARATOR
                                                + std::to_string(p_fd->tunk_size));

        /* 2、阻塞等待服务端的文件ID，等待时间BLOCKING_TIMES秒 */
        if (!(p_fd->block_cv).wait_for(lck, std::chrono::milliseconds(BLOCKING_TIMES), [&p_fd] () {return p_fd->flag_recv_fileid;})
            && !(p_fd->flag_recv_fileid))
        {
            throw std::runtime_error("未在规定时间内接收到文件ID.");
        }

        /* 3、传输块数据： ${cmd:3}${file_id}-${tunk_id}:${tunk_data} */
        for (auto tunk_id = 0; tunk_id < p_fd->tunk_data.size(); tunk_id++)
        {
            trans_bytes += _write_bytes(CMD_TUNKDATA, _id_format_transfomer(p_fd->file_id)
                                                    + _id_format_transfomer(tunk_id)
                                                    + PROTOCOL_SEPARATOR
                                                    + p_fd->tunk_data[tunk_id]);
        }

        /* 4、阻塞等待服务端的ACK或RETRANS，等待时间3秒 */
        if (!(p_fd->block_cv).wait_for(lck, std::chrono::milliseconds(BLOCKING_TIMES), [&p_fd] () {return p_fd->flag_recv_ack || p_fd->flag_recv_retrans;}))
        {
            throw std::runtime_error("未在规定时间内接收到ACK或RETRANS.");
        }

        /* 5、检测是否需要重传：*/
        if (p_fd->flag_recv_retrans)
        {
            for (auto tunk_id : p_fd->retrans_tunk_id)
            {
                trans_bytes += _write_bytes(CMD_TUNKDATA, _id_format_transfomer(p_fd->file_id)
                                                        + _id_format_transfomer(tunk_id)
                                                        + PROTOCOL_SEPARATOR
                                                        + p_fd->tunk_data[tunk_id]);
            }

            if (!(p_fd->block_cv).wait_for(lck, std::chrono::milliseconds(BLOCKING_TIMES), [&p_fd] () {return p_fd->flag_recv_ack;}))
            {
                throw std::runtime_error("重传后未在规定时间内接收到ACK.");
            }
        }

        /* 6、检测是否发送成功 */
        if (p_fd->flag_recv_ack)
        {
            log_info("SerialFTP({})成功传输文件{}", m_portname, file_full_path);
        }
    }
    catch(const std::exception& e)
    {
        log_error("SerialFTP({})传输文件{})失败, error msg = {}", m_portname, file_full_path, e.what());
        goto err_ret;
    }

err_ret:
    /* 释放资源 */
    {
        std::unique_lock<std::mutex> lck(c_file_management_lock);
        c_file_management.erase(file_name);
    }
    
    return std::pair<bool, size_t>(ret, trans_bytes);
}

SerialFileTransfer::ftret_type SerialFileTransfer::receive(str_type file_full_path)
{
    std::size_t recv_bytes = 0;
    std::size_t trans_bytes = 0;
    bool ret = true;

    try
    {
        /* 获取文件ID */
        std::size_t file_id;
        if (!s_readable_fileid.dequeue(file_id, false))
        {
            throw std::runtime_error("不存在可读文件");
        }

        /* 获取文件描述符 */
        std::unique_lock<std::mutex> _lock(s_file_management_lock);
        if (s_file_management.find(file_id) == s_file_management.end())
        {
            throw std::runtime_error("不存在该文件ID");
        }

        /* 将数据写入文件 */
        FileReceDescription &file_desc = s_file_management[file_id];
        std::size_t rep_filename_pos = file_full_path.find(REP_FILENAME_SYMBOL);
        if (rep_filename_pos == std::string::npos)
        {
            throw std::runtime_error("Can't find replace FILENAME symbol.");
        }
        file_full_path.replace(rep_filename_pos, REP_FILENAME_SYMBOL.size(), file_desc.file_name);
        // 打开写入文件
        std::ofstream ofs(file_full_path, std::ios::binary | std::ios::trunc);
        if (!ofs.is_open())
        {
            throw std::runtime_error("File not existed.");
        }
        for(auto &it : file_desc.tunk_data)
        {
            ofs.write((it.second).c_str(), (it.second).size());
        }

        log_info("SerialFTP({})成功接收文件({})", m_portname, file_full_path);

        /* 释放资源 */
        ofs.flush();
        ofs.close();
        s_fileid_allocator.enqueue(file_id);
        s_file_management.erase(file_id);
    }
    catch(const std::exception& e)
    {
        log_error("SerialFTP({})接收文件异常, msg = {}", m_portname, e.what());
    }
    

err_ret:
    return std::pair<bool, size_t>(ret, recv_bytes);
}

/**********************************************************************************
 *************************    Sub Thread    ***************************************
 **********************************************************************************/
// 创建子线程监听客户端数据
void SerialFileTransfer::_subthread_listen_client()
{
    try
    {
        std::vector<uint8_t> rbuf(PROTOCOL_LEN);
        for(;;)
        {
            /* 阻塞读取数据 */
            std::size_t recv_bytes = boost::asio::read(*p_serialport, boost::asio::buffer(rbuf.data(), PROTOCOL_LEN));

            log_debug("SerialFTP({})接收到一包数据({})", m_portname, _convert_vecu8_to_hexstring(rbuf));

            /* 校验 */
            PROTOCOL_CMD cmd;
            try
            {
                _check_protocol_frame(rbuf, cmd);
            }
            catch(const std::exception& e)
            {
                log_error("SerialFTP({})接收到一包错误数据({}), msg = {}", m_portname, _convert_vecu8_to_hexstring(rbuf), e.what());
                continue;
            }

            /* 去除起始位和停止位 */
            std::vector<uint8_t> payload(rbuf.begin() + PROTOCOL_SB_LEN + PROTOCOL_CMD_LEN, 
                                         rbuf.begin() + PROTOCOL_SB_LEN + PROTOCOL_CMD_LEN + PROTOCOL_PAYLOAD_LEN);

            /* 处理协议包 */
            ProtocolPackage package(cmd, payload);
            _handle_protocol_package(package);
        }
    }
    catch(const std::exception& e)
    {
        log_error("SerialFTP({})监听客户端连接出错, msg = {}", m_portname, e.what());
    }
}

// 创建子线程监听服务器文件管理
void SerialFileTransfer::_subthread_listen_manage_server_filedescription()
{
    try
    {
        for(;;)
        {
            {
                std::unique_lock<std::mutex> lock(s_file_management_lock);
                auto cur_time = std::chrono::system_clock::now();       // 当前时间
                std::vector<std::size_t> to_delete_fileid;              // 重传超出次数需要删除的file_id
                for(auto &it : s_file_management)
                {
                    auto &file_id   = it.first;
                    auto &file_desc = it.second;
                    auto &file_name = file_desc.file_name;
                    auto &tunk_data = file_desc.tunk_data;
                    if (   tunk_data.size() < file_desc.tunk_size 
                        && std::chrono::duration_cast<std::chrono::milliseconds>(cur_time - file_desc.recv_time).count() > MAXIMUM_WAIT_TIMES
                        )
                    {
                        log_warning("SerialFTP({})检测到文件({}:{})接收超时", m_portname, file_id, file_name);

                        /* 查找需要重传的数据块 */
                        std::vector<std::size_t> retrans_tunk_id;
                        for(std::size_t tunk_id = 0; tunk_id < file_desc.tunk_size; tunk_id++)
                        {
                            if (tunk_data.find(tunk_id) == tunk_data.end())
                            {
                                retrans_tunk_id.emplace_back(tunk_id);
                            }
                        }

                        /* 发送重传数据包 */
                        if (retrans_tunk_id.size() > 0)
                        {
                            std::string write_str = _id_format_transfomer(file_id) + PROTOCOL_SEPARATOR;
                            for(auto idx = 0; idx < retrans_tunk_id.size(); idx++)
                            {
                                write_str += _id_format_transfomer(retrans_tunk_id[idx]);
                                if (idx != (retrans_tunk_id.size() - 1))
                                {
                                    write_str += PROTOCOL_SEPARATOR;
                                }
                            }
                            _write_bytes(CMD_RETRANS, write_str);
                        }

                        /* 记录重传次数 */
                        file_desc.retrans_cnt++;
                        if (file_desc.retrans_cnt > MAXIMUM_RETRANS_CNT)
                        {
                            to_delete_fileid.emplace_back(file_id);
                            log_error("SerialFTP({})重传({}:{})超过最大次数", m_portname, file_id, file_name);
                        }
                    }
                }

                /* 删除超出重传次数的file id */
                for(auto file_id : to_delete_fileid)
                {
                    s_fileid_allocator.enqueue(file_id);
                    s_file_management.erase(file_id);
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(SERVER_LISTEN_INTERVAL));
        }
    }
    catch(const std::exception& e)
    {
        log_error("SerialFTP({})监听服务器文件管理异常, msg = {}", m_portname, e.what());
    }
}

/**********************************************************************************
 *************************    Private    ******************************************
 **********************************************************************************/
// 处理协议数据包
SerialFileTransfer::ftret_type SerialFileTransfer::_handle_protocol_package(ProtocolPackage &package)
{
    std::size_t recv_bytes = 0;
    std::size_t trans_bytes = 0;
    bool ret = true;

    /* 根据控制位处理数据 */
    if      (package.cmd == CMD_FILEINFO)   // ! 文件信息
    {
        /* 解析文件信息，获取file_name\file_size\tunk_size */
        std::string file_name;
        std::size_t file_size, tunk_size;
        try
        {
            _parse_fileinfo_from_payload(package.payload, file_name, file_size, tunk_size);
            log_debug("SerialFTP({})接收到文件信息({}:{}:{})", m_portname, file_name, file_size, tunk_size);
        }
        catch(const std::exception& e)
        {
            log_error("SerialFTP({})解析FILEINFO的Payload({})出错, msg = {}", m_portname, _convert_vecu8_to_hexstring(package.payload), e.what());
            goto err_ret;
        }

        /* 分配文件ID */
        std::size_t file_id;
        if (!s_fileid_allocator.dequeue(file_id, false))
        {
            log_error("SerialFTP({})分配文件ID失败", m_portname);
            goto err_ret;
        }

        /* 创建文件描述符并加入文件管理 */
        {
            std::unique_lock<std::mutex> lock(s_file_management_lock);
            s_file_management[file_id] = FileReceDescription(file_id, file_name, file_size, tunk_size);
        }
        
        /* 向客户端发送文件ID */
        trans_bytes += SerialFileTransfer::_write_bytes(CMD_FILEID, file_name + PROTOCOL_SEPARATOR + _id_format_transfomer(file_id));
    }
    else if (package.cmd == CMD_FILEID)     // ! 文件ID
    {
        /* 解析文件ID，获取file_name\file_id */
        std::string file_name;
        std::size_t file_id;
        try
        {
            _parse_fileid_from_payload(package.payload, file_name, file_id);
            log_debug("SerialFTP({})接收到文件ID({}:{})", m_portname, file_id, file_name);
        }
        catch(const std::exception& e)
        {
            log_error("SerialFTP({})解析FILEID的Payload({})出错, msg = {}", m_portname, _convert_vecu8_to_hexstring(package.payload), e.what());
            goto err_ret;
        }

        {
            std::unique_lock<std::mutex> lck(c_file_management_lock);
            /* 查找客户端阻塞条件管理器，置位变量 */ 
            if (c_file_management.find(file_name) == c_file_management.end())
            {
                log_error("SerialFTP({})接收到文件ID({})到未在客户端管理器中查询到条目", m_portname, file_id);
                goto err_ret;
            }

            {
                std::unique_lock<std::mutex> lck(c_file_management[file_name]->block_lock);
                c_file_management[file_name]->file_id = file_id;
                c_file_management[file_name]->flag_recv_fileid = true;
                c_file_management[file_name]->block_cv.notify_all();
            }
        }
    }
    else if (package.cmd == CMD_TUNKDATA)   // ! 块数据
    {
        /* 解析块数据，获取file_id\tunk_id\tunk_data */
        std::size_t file_id, tunk_id;
        std::string tunk_data;
        try
        {
            _parse_tunkdata_from_payload(package.payload, file_id, tunk_id, tunk_data);
            log_debug("SerialFTP({})接收到块数据({}:{}:{})", m_portname, file_id, tunk_id, tunk_data);
        }
        catch(const std::exception& e)
        {
            log_error("SerialFTP({})解析CMD_TUNKDATA的Payload({})出错, msg = {}", m_portname, _convert_vecu8_to_hexstring(package.payload), e.what());
            goto err_ret;
        }

        /* 检测文件id是否存在以及tunk_id是否已存在或超出tunk_size */
        try
        {
            std::unique_lock<std::mutex> lock(s_file_management_lock);

            if (s_file_management.find(file_id) == s_file_management.end())
            {
                throw std::runtime_error("未知file_id");
            }

            auto &file_desc = s_file_management[file_id];
            if (tunk_id >= file_desc.tunk_size)
            {
                throw std::runtime_error("未知的tunk_id");
            }

            std::map<std::size_t, std::string> &tunk_mp = file_desc.tunk_data;
            if (tunk_mp.find(tunk_id) != tunk_mp.end())
            {
                throw std::runtime_error("重复的tunk_id");
            }

            /* 将块数据存入文件管理 */
            file_desc.recv_time = std::chrono::system_clock::now();
            tunk_mp[tunk_id] = tunk_data; 

            /* 判断块数据是否收集完成 */
            if (tunk_mp.size() == file_desc.tunk_size)
            {
                /* 发送ACK */
                trans_bytes += SerialFileTransfer::_write_bytes(CMD_ACK, _id_format_transfomer(file_id));

                s_readable_fileid.enqueue(file_id);

                /* 回调通知上层接口接收到完整文件 */
                f_readable_cb();
            }
        }
        catch(const std::exception& e)
        {
            log_error("SerialFTP({})接收数据块({})出错, msg = {}", m_portname, _convert_vecu8_to_hexstring(package.payload), e.what());
            goto err_ret;
        }
    }
    else if (package.cmd == CMD_ACK)        // ! ack数据
    {
        /* 解析ACK数据，获取file_id */
        std::size_t file_id;
        try
        {
            _parse_ack_from_payload(package.payload, file_id);
            log_debug("SerialFTP({})接收到ACK({})", m_portname, file_id);
        }
        catch(const std::exception& e)
        {
            log_error("SerialFTP({})解析ACK的Payload({})出错, msg = {}", m_portname, _convert_vecu8_to_hexstring(package.payload), e.what());
            goto err_ret;
        }

        /* 从客户端文件管理器中获取transfer线程阻塞的条件变量，通知其解锁 */
        {
            std::unique_lock<std::mutex> lck(c_file_management_lock);

            for (auto &it : c_file_management)
            {
                if ((it.second)->file_id == file_id)
                {
                    std::unique_lock<std::mutex> lck(c_file_management[it.first]->block_lock);
                    c_file_management[it.first]->flag_recv_ack = true;
                    c_file_management[it.first]->block_cv.notify_all();
                    break;
                }
            }
        }

    }
    else if (package.cmd == CMD_RETRANS)    // ! 重传
    {
        /* 解析重传数据，获取file_id和需要重传的块数据 */
        std::size_t file_id;
        std::vector<std::size_t> retrans_tunk_id;
        try
        {
            _parse_retrans_from_payload(package.payload, file_id, retrans_tunk_id);
            log_debug("SerialFTP({})接收到重传({}:{})", m_portname, file_id, retrans_tunk_id.size());
        }
        catch(const std::exception& e)
        {
            log_error("SerialFTP({})解析RETRANS的Payload({})出错, msg = {}", m_portname, _convert_vecu8_to_hexstring(package.payload), e.what());
            goto err_ret;
        }

        /* 从客户端文件管理器中获取transfer线程阻塞的条件变量，通知其解锁 */
        {
            std::unique_lock<std::mutex> lck(c_file_management_lock);

            for (auto &it : c_file_management)
            {
                if ((it.second)->file_id == file_id)
                {
                    std::unique_lock<std::mutex> lck(c_file_management[it.first]->block_lock);
                    c_file_management[it.first]->flag_recv_retrans = true;
                    c_file_management[it.first]->retrans_tunk_id = retrans_tunk_id;
                    c_file_management[it.first]->block_cv.notify_all();
                    break;
                }
            }
        }
    }

err_ret:
    return std::pair<bool, size_t>(ret, recv_bytes);
}

// 转化协议帧为十六进制字符串
void SerialFileTransfer::_check_protocol_frame(std::vector<uint8_t> &frame, PROTOCOL_CMD &cmd)
{
    /* 校验块大小 */
    if (frame.size() != PROTOCOL_LEN) 
    {
        throw std::runtime_error("Error tunk size.");
    }

    /* 校验起始位 */
    for(int idx = 0; idx < PROTOCOL_SB_LEN; idx++) 
    {
        if (frame[idx] != PROTOCOL_START_BITS[idx]) 
        {
            throw std::runtime_error("Error start bits.");
        }
    }

    /* 校验控制位 */
    cmd = static_cast<PROTOCOL_CMD>(frame[PROTOCOL_SB_LEN]);
    if (cmd != CMD_FILEINFO && cmd != CMD_FILEID && cmd != CMD_TUNKDATA && cmd != CMD_ACK && cmd != CMD_RETRANS) 
    {
        throw std::runtime_error("Error cmd bits.");
    }

    /* 校验停止位 */
    for(int idx = 0; idx < PROTOCOL_EB_LEN; idx++) 
    {
        if (frame[PROTOCOL_LEN - PROTOCOL_EB_LEN + idx] != PROTOCOL_END_BITS[idx]) 
        {
            throw std::runtime_error("Error end bits.");
        }
    }
}

// 校验协议帧
std::string SerialFileTransfer::_convert_vecu8_to_hexstring(std::vector<uint8_t> &frame)
{
    std::stringstream ss;
    auto size = frame.size();
    for(auto idx = 0; idx < size; idx++) 
    {
        ss << "0x" << std::hex << static_cast<int>(frame[idx]);
        if (idx != size - 1) 
        {
            ss << " ";
        }
    }
    return ss.str();
}

// 解析文件信息,"${file_name}-${file_size}-${tunk_size}"
void SerialFileTransfer::_parse_fileinfo_from_payload(std::vector<uint8_t> &payload, std::string &file_name, std::size_t &file_size, std::size_t &tunk_size)
{
    /* 解析字符串 */
    int cnt_separator = 0;
    std::vector<std::string> arr;
    _separate_payload_by_symbol(payload, arr, cnt_separator);

    /* 检查格式是否规范 */
    if ((arr.size() != 3) || (cnt_separator != 2))
    {
        throw std::runtime_error("Error payload format.");
    }

    file_name = arr[0];
    file_size = std::stoull(arr[1]);
    tunk_size = std::stoull(arr[2]);
}

// 解析文件ID,"${file_name}-${file_id}"
void SerialFileTransfer::_parse_fileid_from_payload(std::vector<uint8_t> &payload, std::string &file_name, std::size_t &file_id)
{
    /* 解析字符串 */
    int cnt_separator = 0;               // 折号出现次数
    std::vector<std::string> arr;
    _separate_payload_by_symbol(payload, arr, cnt_separator);

    /* 检查格式是否规范 */
    if ((arr.size() != 2) || (cnt_separator != 1))
    {
        throw std::runtime_error("Error payload format.");
    }

    file_name = arr[0];
    if (arr[1].size() != PROTOCOL_FILEID_LEN)
    {
        throw std::runtime_error("Error file id length.");
    }
    file_id = std::stoull(arr[1]);
}

// 解析块数据，"${file_id}${tunk_id}-${tunk_data}"
void SerialFileTransfer::_parse_tunkdata_from_payload(std::vector<uint8_t> &payload, std::size_t &file_id, std::size_t &tunk_id, std::string &tunk_data)
{
    /* 解析字符串 */
    int cnt_separator = 0;
    std::vector<std::string> arr;
    _separate_payload_by_symbol(payload, arr, cnt_separator);

    /* 检查格式是否规范 */
    if ((arr.size() != 2) || (cnt_separator != 1))
    {
        throw std::runtime_error("Error payload format.");
    }

    if (arr[0].size() != (PROTOCOL_FILEID_LEN + PROTOCOL_TUNKID_LEN))
    {
        throw std::runtime_error("Error file/tunk id length.");
    }

    file_id = std::stoull(arr[0].substr(0, PROTOCOL_FILEID_LEN));
    tunk_id = std::stoull(arr[0].substr(PROTOCOL_FILEID_LEN, PROTOCOL_TUNKID_LEN));
    tunk_data = arr[1];
}

// 解析ACK数据,"${file_id}"
void SerialFileTransfer::_parse_ack_from_payload(std::vector<uint8_t> &payload, std::size_t &file_id)
{
    /* 解析字符串 */
    int cnt_separator = 0;
    std::vector<std::string> arr;
    _separate_payload_by_symbol(payload, arr, cnt_separator);

    /* 检查格式是否规范 */
    if ((arr.size() != 1) || (cnt_separator != 0))
    {
        throw std::runtime_error("Error payload format.");
    }

    if (arr[0].size() != PROTOCOL_FILEID_LEN)
    {
        throw std::runtime_error("Error file id length.");
    }
    file_id = std::stoull(arr[0]);
}

// 解析重传数据,"${file_id}-${retrans_tunk_id}-${id2}-{id3}"
void SerialFileTransfer::_parse_retrans_from_payload(std::vector<uint8_t> &payload, std::size_t &file_id, std::vector<std::size_t> &retrans_tunk_id)
{
    /* 解析字符串 */
    int cnt_separator = 0;
    std::vector<std::string> arr;
    _separate_payload_by_symbol(payload, arr, cnt_separator);

    /* 检查格式是否规范 */
    if ( arr.size() != (cnt_separator + 1) )
    {
        throw std::runtime_error("Error payload format.");
    }

    if (arr[0].size() != PROTOCOL_FILEID_LEN)
    {
        throw std::runtime_error("Error file id length.");
    }
    file_id = std::stoull(arr[0]);
    for(auto idx = 1; idx < arr.size(); idx++)
    {
        if (arr[idx].size() != PROTOCOL_TUNKID_LEN)
        {
            throw std::runtime_error("Error tunk id length.");
        }
        retrans_tunk_id.emplace_back(std::stoull(arr[idx]));
    }
}

// 通过'-','\x00'等符号分隔payload
void SerialFileTransfer::_separate_payload_by_symbol(std::vector<uint8_t> &payload, std::vector<std::string> &parse, int &cnt_separator)
{
    /* 根据分隔符划分 */
    std::string tmp = "";
    for(auto &ch : payload)
    {
        if (ch == PROTOCOL_SEPARATOR_CH) {
            cnt_separator++;
            parse.emplace_back(tmp);
            tmp = "";
        } else if (ch == PROTOCOL_NULL_CH) {
            break;
        } else {
            tmp += ch;
        }
    }
    parse.emplace_back(tmp);  // 尾部处理
}

// 写入一帧数据，payload长度不能超过PROTOCOL_PAYLOAD_LEN
std::size_t SerialFileTransfer::_write_bytes(PROTOCOL_CMD cmd, std::vector<uint8_t> payload)
{
    std::size_t trans_bytes = 0;
    std:size_t payload_size = payload.size();

    /* 检测payload是否超过最大长度 */
    if (payload_size > PROTOCOL_PAYLOAD_LEN)
    {
        log_error("SerialFTP({})写入一帧数据时payload({})超出最大限制", m_portname, _convert_vecu8_to_hexstring(payload));
        return trans_bytes;
    }

    /* 构建完整帧协议 */
    std::vector<uint8_t> frame;

    // 添加起始位
    for(auto idx = 0; idx < PROTOCOL_SB_LEN; idx++)
    {
        frame.emplace_back(PROTOCOL_START_BITS[idx]);
    }

    // 添加控制位
    frame.emplace_back(static_cast<uint8_t>(cmd));

    // 添加负载
    for(int idx = 0; idx < payload_size; idx++)
    {
        frame.emplace_back(payload[idx]);
    }

    // 添加补'\x00'
    std::size_t zerofill_size = PROTOCOL_PAYLOAD_LEN - payload_size;
    for(int idx = 0; idx < zerofill_size; idx++)
    {
        frame.emplace_back('\x00');
    }

    // 添加结束位
    for(int idx = 0; idx < PROTOCOL_EB_LEN; idx++)
    {
        frame.emplace_back(PROTOCOL_END_BITS[idx]);
    }

    /* 串口发送数据 */
    trans_bytes += boost::asio::write(*p_serialport, boost::asio::buffer(frame.data(), PROTOCOL_LEN));

    log_debug("SerialFTP({})发送到一包数据({})", m_portname, _convert_vecu8_to_hexstring(frame));

    return trans_bytes;
}

std::size_t SerialFileTransfer::_write_bytes(PROTOCOL_CMD cmd, std::string payload)
{
    std::vector<uint8_t> payload_u(payload.begin(), payload.end());
    return _write_bytes(cmd, payload_u);
}

// 将file_id/tunk_id转化为固定长度的字符串
std::string SerialFileTransfer::_id_format_transfomer(std::size_t id)
{
    std::string str = std::to_string(id);
    return (str.size() < 2 ? "0" + str : str);
}



