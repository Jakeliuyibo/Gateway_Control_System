/*
 * @Author: zn zhangn12581@163.com
 * @Date: 2023-11-08 14:57:51
 * @LastEditors: zn zhangn12581@163.com
 * @LastEditTime: 2023-11-08 15:10:03
 * @FilePath: /Gateway_Control_System/driver/serial/serial.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "serial.h"

using namespace utility;
using namespace driver;
using namespace boost::asio;



Serial::Serial(IniConfigParser *parser)
{
    bool parserFlag = true;

    /* 解析参数 */
    std::string serial_port;
    int serial_baudrate; 
    parserFlag &= parser->getValue<std::string>("SERIAL", "SERIAL_PORT"    , serial_port);
    parserFlag &= parser->getValue<int        >("SERIAL", "SERIAL_BAUDRATE", serial_baudrate);

    if(!parserFlag)
    {
        log_error("Init serial failed, when load config");
        return;
    }

    /* 初始化串口 */
    p_serialport = new boost::asio::serial_port(m_ios);
    if (p_serialport)
    {
		if(!open(serial_port, serial_baudrate))
        {
            log_error("Can't open serial {}", serial_port);
        }
	}
    log_warning("init serial");
}

Serial::~Serial()
{
    close();
}

bool Serial::open(const std::string &port, int bard_rate)
{
	if (!p_serialport)
    {
		return false;
	}
 
    //Open Serial port object
    p_serialport->open(port, ec);
	
	//Set port argument
	p_serialport->set_option(serial_port::baud_rate(bard_rate), ec);
	p_serialport->set_option(serial_port::flow_control(serial_port::flow_control::none), ec);
	p_serialport->set_option(serial_port::parity(serial_port::parity::none), ec);
	p_serialport->set_option(serial_port::stop_bits(serial_port::stop_bits::one), ec);
	p_serialport->set_option(serial_port::character_size(8), ec);
 
	return true;
}


void Serial::read()
{
    // boost::asio::read();
}


void Serial::write(const std::string &data)
{
    size_t len = boost::asio::write(*p_serialport, buffer(data), ec);
    log_warning("Serial write data {}", data);
}

void Serial::close()
{
    if(p_serialport)
    {
        p_serialport->close();
        delete p_serialport;
        p_serialport = nullptr;
    }
}

// TODO error_code

//看一下底层   buffer(data)   delete  set_option    p_serialport->open(port, ec);
//设置一下cmake
//百度一下文件搜不到的问题