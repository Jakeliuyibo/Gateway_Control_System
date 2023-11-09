#include "serial.h"

using namespace utility;
using namespace driver;
using namespace boost::asio;

Serial::Serial(IniConfigParser *parser)
{
    bool parserFlag = true;

    /* 解析参数 */
    parserFlag &= parser->getValue<std::string>("SERIAL", "SERIAL_PORT"    , m_portname);
    parserFlag &= parser->getValue<int        >("SERIAL", "SERIAL_BAUDRATE", m_baudrate);
    if(!parserFlag)
    {
        log_error("Init serial failed, when load config");
        return;
    }

    /* 初始化串口 */
    p_serialport = new boost::asio::serial_port(m_ios);
    if (p_serialport)
    {
		if(!open())
        {
            log_error("Can't open serial {}", m_portname);
            return;
        }
	}
}

Serial::~Serial()
{
    close();
}

bool Serial::open()
{
    boost::system::error_code ec;

	if (!p_serialport)
    {
		return false;
	}
 
    /* open Serial port object */
    p_serialport->open(m_portname, ec);
	
	/* set port argument       */
	p_serialport->set_option(serial_port::baud_rate(m_baudrate), ec);
	p_serialport->set_option(serial_port::flow_control(serial_port::flow_control::none), ec);
	p_serialport->set_option(serial_port::parity(serial_port::parity::none), ec);
	p_serialport->set_option(serial_port::stop_bits(serial_port::stop_bits::one), ec);
	p_serialport->set_option(serial_port::character_size(8), ec);
    if(ec.value() != 0)
    {
        log_error("Open serial {} failed, error info = {}", m_portname, ec.message());
        return false;
    }

	return true;
}


void Serial::read()
{
    // boost::asio::read();
}

void Serial::write(const std::string &data)
{
    boost::system::error_code ec;

    size_t len = boost::asio::write(*p_serialport, buffer(data), ec);
    if(ec.value() != 0)
    {
        log_error("Write data to serial {} failed, error info = {}", m_portname, ec.message());
        return;
    }
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