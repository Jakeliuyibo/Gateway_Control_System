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