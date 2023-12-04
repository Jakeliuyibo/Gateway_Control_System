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
    std::thread(
        [this] ()
        {
            try
            {
                std::vector<uint8_t> rbuf(PROTOCOL_LEN);
                for(;;)
                {
                    /* 阻塞读取数据 */
                    std::size_t recv_bytes = boost::asio::read(*p_serialport, boost::asio::buffer(rbuf.data(), PROTOCOL_LEN));

                    /* 校验 */
                    PROTOCOL_CMD cmd;
                    try
                    {
                        _check_protocol_frame(rbuf, cmd);
                    }
                    catch(const std::exception& e)
                    {
                        log_error("SerialFileTransfer({})接收到一包错误数据({}), msg = {}", m_portname, _convert_vecu8_to_hexstring(rbuf), e.what());
                        continue;
                    }

                    /* 去除起始位和停止位 */
                    std::vector<uint8_t> payload(rbuf.begin() + PROTOCOL_SB_LEN + PROTOCOL_CMD_LEN, 
                                                 rbuf.begin() + PROTOCOL_SB_LEN + PROTOCOL_CMD_LEN + PROTOCOL_PAYLOAD_LEN);

                    ProtocolPackage package(cmd, payload);

                    /* 将数据存入队列缓存 */
                    s_rbuf.enqueue(package);

                    /* 回调通知上层接口数据可读 */
                    f_readable_cb();
                }
            }
            catch(const std::exception& e)
            {
                log_error("SerialFileTransfer({})监听客户端连接出错, msg = {}", m_portname, e.what());
            }
            
        }
    ).detach();
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
    std::size_t recv_bytes = 0;
    std::size_t trans_bytes = 0;
    bool ret = true;

    /* 读取文件信息 */
    try
    {
        // 打开文件并获取文件大小
        std::ifstream ifs(file_full_path, std::ios::binary | std::ios::ate);
        auto file_size = ifs.tellg();
        ifs.seekg(0, std::ios::beg);

        // 计算分包情况

    }
    catch(const std::exception& e)
    {
        log_error("SerialFileTransfer({})获取文件file({})失败, error msg = {}", m_portname, file_full_path, e.what());
    }
    

    /* 计算分包情况 */

    /* 传输file_name-file_size-tunk_size */

    /* 等待服务器file_id */

    /* 传输file_id-tunk_id:tunk_data */
    
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
    std::size_t trans_bytes = 0;
    bool ret = true;

    /* 从缓存获取一条数据 */
    ProtocolPackage package;
    if (!s_rbuf.dequeue(package, false))
    {
        log_error("SerialFileTransfer({})缓存不存在可读数据", m_portname);
        goto ret;
    }

    /* 根据控制位处理数据 */
    if (package.cmd == CMD_FILEINFO)        // ! 文件信息
    {
        /* 解析文件信息，获取file_name\file_size\tunk_size */
        std::string file_name;
        std::size_t file_size;
        std::size_t tunk_size;
        try
        {
            _parse_fileinfo_from_payload(package.payload, file_name, file_size, tunk_size);
        }
        catch(const std::exception& e)
        {
            log_error("SerialFileTransfer({})解析Payload({})出错, msg = {}", m_portname, _convert_vecu8_to_hexstring(package.payload), e.what());
            goto ret;
        }

        /* 分配文件ID */
        std::size_t file_id;
        if (!s_fileid_allocator.dequeue(file_id, false))
        {
            log_error("SerialFileTransfer({})分配文件ID失败", m_portname);
            goto ret;
        }

        /* 创建文件描述符并加入文件管理 */
        FileDescription file_desc(file_id, file_name, file_size, tunk_size);
        {
            std::unique_lock<std::mutex> lock(s_file_management_lock);
            s_file_management[file_id] = file_desc; // TODO 修改为动态资源
        }
        
        /* 向客户端发送文件ID */
        trans_bytes += SerialFileTransfer::_write_bytes(CMD_FILEID, std::vector<uint8_t>(1, static_cast<uint8_t>(file_id)));
    }
    else if (package.cmd == CMD_TUNKDATA)   // ! 块数据
    {
        /* 解析块数据，获取file_id\tunk_id\tunk_data */

        /* 将块数据存入文件管理 */

        /* 判断块数据是否收集完成，如果完成发送ACK，并释放文件ID */

    }
    else if (package.cmd == CMD_ACK)        // ! ack数据
    {
        /* 解析ACK数据，获取file_id */

        /* 遍历c_file_management，释放资源 */

        /* 从set中获取transfer线程阻塞的条件变量，通知其解锁 */
    }
    else if (package.cmd == CMD_RETRANS)    // ! 重传
    {
        /* 解析重传数据，获取file_id和需要重传的块数据 */

        /* 遍历c_file_management */

        /* 从set中获取transfer线程阻塞的条件变量，通知其解锁 */
    }

ret:
    return std::pair<bool, size_t>(ret, recv_bytes);
}

/**********************************************************************************
 *************************    Private    ******************************************
 **********************************************************************************/
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
    int cntDashes = 0;               // 折号出现次数
    int cntSemicolons = 0;           // 分号出现次数
    std::vector<std::string> arr;
    _separate_payload_by_symbol(payload, arr, cntDashes, cntSemicolons);

    /* 检查格式是否规范 */
    if ((arr.size() != 3) || (cntDashes != 2))
    {
        throw std::runtime_error("Error payload format.");
    }

    file_name = arr[0];
    file_size = std::stoull(arr[1]);
    tunk_size = std::stoull(arr[2]);
}

// 解析块数据，"${file_id}-${tunk_id}:${tunk_data}"
void SerialFileTransfer::_parse_tunkdata_from_payload(std::vector<uint8_t> &payload, std::size_t &file_id, std::size_t &tunk_id, std::string &tunk_data)
{
    /* 解析字符串 */
    int cntDashes = 0;               // 折号出现次数
    int cntSemicolons = 0;           // 分号出现次数
    std::vector<std::string> arr;
    _separate_payload_by_symbol(payload, arr, cntDashes, cntSemicolons);

    /* 检查格式是否规范 */
    if ((arr.size() != 3) || (cntDashes != 1) || (cntSemicolons != 1))
    {
        throw std::runtime_error("Error payload format.");
    }

    file_id = std::stoull(arr[0]);
    tunk_id = std::stoull(arr[1]);
    tunk_data = arr[2];
}

// 解析ACK数据,"${file_id}"
void SerialFileTransfer::_parse_ackdata_from_payload(std::vector<uint8_t> &payload, std::size_t &file_id)
{
    /* 解析字符串 */
    int cntDashes = 0;               // 折号出现次数
    int cntSemicolons = 0;           // 分号出现次数
    std::vector<std::string> arr;
    _separate_payload_by_symbol(payload, arr, cntDashes, cntSemicolons);

    /* 检查格式是否规范 */
    if ((arr.size() != 1) || (cntDashes != 0) || (cntSemicolons != 0))
    {
        throw std::runtime_error("Error payload format.");
    }

    file_id = std::stoull(arr[0]);
}

// 解析重传数据,"${file_id}-${retrans_tunk_id}-${id2}-{id3}"
void SerialFileTransfer::_parse_retrans_from_payload(std::vector<uint8_t> &payload, std::size_t &file_id, std::vector<std::size_t> &retrans_tunk_id)
{
    /* 解析字符串 */
    int cntDashes = 0;               // 折号出现次数
    int cntSemicolons = 0;           // 分号出现次数
    std::vector<std::string> arr;
    _separate_payload_by_symbol(payload, arr, cntDashes, cntSemicolons);

    /* 检查格式是否规范 */
    if ((arr.size() != (cntDashes + 1)) || (cntSemicolons != 0))
    {
        throw std::runtime_error("Error payload format.");
    }

    file_id = std::stoull(arr[0]);
    for(auto idx = 1; idx < arr.size(); idx++)
    {
        retrans_tunk_id.emplace_back(std::stoull(arr[idx]));
    }
}

// 通过'-'\':'\'\x00'等符号分隔payload
void SerialFileTransfer::_separate_payload_by_symbol(std::vector<uint8_t> &payload, std::vector<std::string> &parse, int &cnt_dashes, int &cnt_semicolons)
{
    /* 根据分隔符划分 */
    std::string tmp = "";
    for(auto &ch : payload)
    {
        if (ch == '-') 
        {
            cnt_dashes++;
            parse.emplace_back(tmp);
            tmp = "";
        } 
        else if (ch == ':') 
        {
            cnt_semicolons++;
            parse.emplace_back(tmp);
            tmp = "";
        } 
        else if (ch == '\x00') 
        {
            break;
        } 
        else 
        {
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
        log_error("SerialFileTransfer({})写入一帧数据时payload({})超出最大限制", m_portname, _convert_vecu8_to_hexstring(payload));
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
    log_critical("串口发送{}", _convert_vecu8_to_hexstring(frame));
    trans_bytes += boost::asio::write(*p_serialport, boost::asio::buffer(frame.data(), PROTOCOL_LEN));

    return trans_bytes;
}










