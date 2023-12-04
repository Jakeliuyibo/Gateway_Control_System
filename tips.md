## 虚拟串口
调试时使用socat虚拟串口：
    串口-串口    ：  socat -d -d pty,b115200,raw,echo=0 pty,b115200,raw,echo=0
    串口-TCP服务器:  socat -d -d pty,b115200,raw,nonblock,ignoreeof,cr,echo=0 TCP4-LISTEN:2234,reuseaddr
    串口-TCP客户端： socat -d -d pty,b115200,raw,nonblock,ignoreeof,cr,echo=0  TCP:localhost:1234

    CMD_FILEINFO = 0x01,     // 文件信息    : echo -e -n "\x69\x96\x01a-1-1\x0D\x0A" > /dev/pts/2
    CMD_FILEID   = 0x02,     // 文件ID      : 
    CMD_TUNKDATA = 0x03,     // 块数据      :
    CMD_ACK      = 0x04,     // ACK信息     :
    CMD_RETRANS  = 0x05      // 重传块数据  :

## 启动rabbitmq-server服务
service rabbitmq-server restart

## 十六进制查看文件
    root@ hexdump -C 2