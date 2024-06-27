# Gateway_Control_System

# 安装和运行

## Software
* vscode(cmake\git\c++\gdb\Remote Development等插件)
* git
* docker

## Docker
* 安装HYPER-V、WSL2、Docker Desktop
* 配置镜像源
  ```
    "registry-mirrors": [
        "https://registry.docker-cn.com",
        "https://docker.mirrors.ustc.edu.cn"
    ]
  ```
* 安装镜像
  * ubuntu:22.04

* 构建镜像
  ```
  root& docker build -f [${DOCKERFILE_DIR}] -t [${IMAGE_NAME}] .

  for example: root& docker build -f docker/Dockerfile -t ubuntu_cpp:v3 .
  ```

* 启动容器
  ```
  root& docker run -it -P -d --network bridge -v [${PROJECT_DIR}]:[${MAP_DIR}] --name [${ENV_NAME}] [${IMAGE_NAME}]
  
  for example: root& docker run -it -P -d --network bridge -v Y:\Studyplace_Web_Development\Gateway_Control_System:/home/Gateway_Control_System -v Y:\Studyplace_Web_Development\Gateway_Management_System:/home/Gateway_Management_System --name gateway-env-moderncpp --privileged --env "TZ=Asia/Shanghai" --env "NTP_SERVERS=cn.pool.ntp.org"ubuntu_cpp:v3
  ```

* 配置完成，通过vscode附加到运行的容器环境

# 工程目录
```
Gateway_Control_System
├─ 📁.vscode
├─ 📁bin
├─ 📁build
├─ 📁config
│  └─ 📄defconfig.ini
├─ 📁docker
│  └─ 📄Dockerfile
├─ 📁driver
│  ├─ 📁ethernet
│  ├─ 📁opticalfiber
│  ├─ 📁radiodigital
│  ├─ 📁satellite
│  ├─ 📁serial
│  ├─ 📁underwateracoustic
│  └─ 📄功能说明.md
├─ 📁lib
├─ 📁logs
├─ 📁reactor
│  └─ 📄功能需求.md
├─ 📁test
├─ 📁utility
│  ├─ 📁configparser
│  ├─ 📁logger
│  ├─ 📁mysqlpool
│  ├─ 📁rabbitmqclient
│  ├─ 📁singleton
│  ├─ 📁systime
│  ├─ 📁threadpool
│  └─ 📄功能需求.md
├─ 📄.gitignore
├─ 📄CMakeLists.txt
└─ 📄README.md
```

## 命名规范
```
全大写：宏定义
全小写：文件名、命名空间(namespace)
大驼峰：类名、结构体名、类成员函数、普通函数、枚举、类型名(typedef, using)
小驼峰：全局变量、局部变量
其他：
  a、类成员变量：小驼峰+后缀下划线'_'。例如：int a_ = 0;
  b、指针等局部变量：前缀"p"+小驼峰。例如：int *pCnt = nullptr;
  c、常量、静态等局部变量：前缀"k"+小驼峰。例如：const int kCnt = 0; static int kCnt = 0;
  d、条件变量：前缀"is/has"+小驼峰。例如：bool isRecvAck = false;
```

## Tips
### 虚拟串口
调试时使用socat虚拟串口：
    串口-串口    ：  socat -d -d pty,b115200,raw,echo=0 pty,b115200,raw,echo=0
    串口-TCP服务器:  socat -d -d pty,b115200,raw,nonblock,ignoreeof,cr,echo=0 TCP4-LISTEN:2234,reuseaddr
    串口-TCP客户端： socat -d -d pty,b115200,raw,nonblock,ignoreeof,cr,echo=0  TCP:localhost:1234

    CMD_FILEINFO = 0x01,     // 文件信息    : echo -e -n "\x69\x96\x01test.txt-31-3\x00\x00\x0D\x0A" > /dev/pts/2
    CMD_FILEID   = 0x02,     // 文件ID      : echo -e -n "\x69\x96\x02test.txt-01\x00\x00\x00\x00\x0D\x0A" > /dev/pts/2
    CMD_TUNKDATA = 0x03,     // 块数据      : echo -e -n "\x69\x96\x030000-1234567890\x0D\x0A" > /dev/pts/2
                                              echo -e -n "\x69\x96\x030001-1234567890\x0D\x0A" > /dev/pts/2
                                              echo -e -n "\x69\x96\x030002-1\x00\x00\x00\x00\x00\x00\x00\x00\x00\x0D\x0A" > /dev/pts/2
    CMD_ACK      = 0x04,     // ACK信息     : echo -e -n "\x69\x96\x0401\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x0D\x0A" > /dev/pts/2
    CMD_RETRANS  = 0x05      // 重传块数据  : echo -e -n "\x69\x96\x0501-00-01\x00\x00\x00\x00\x00\x00\x00\x0D\x0A" > /dev/pts/2

### 启动rabbitmq-server服务
    root@ service rabbitmq-server restart

### 重启apache服务
    root@ service apache2 restart

### 配置apache服务器
* 配置/etc/apache2/apache2.conf文件
    ServerName localhost:80

* 配置/etc/apache2/sites-available/000-default.conf文件，绑定wsgi接口
    <VirtualHost *:80>
  
        ServerName localhost
      
        DocumentRoot /home/Gateway_Management_System/web
        WSGIScriptAlias / /home/Gateway_Management_System/web/webapp.wsgi
      
        ErrorLog /home/Gateway_Management_System/logs/error.log
        CustomLog /home/Gateway_Management_System/logs/access.log combined
      
        <Directory /home/Gateway_Management_System/web>
            WSGIApplicationGroup %{GLOBAL}
            Require all granted
        </Directory>
  
    </VirtualHost>

* 使能配置 root@ a2ensite 000-default.conf 

* 重启

### 十六进制查看文件
    root@ hexdump -C 2