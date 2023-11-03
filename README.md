# Gateway_Control_System

# 安装和运行

## software

* vscode(cmake\git\c++\gdb\Remote Development等插件)

* git

* docker

## docker

* 安装HYPER-V、WSL2、Docker Desktop

* 配置镜像源
  
  ```
    "registry-mirrors": [
        "https://registry.docker-cn.com",
        "https://docker.mirrors.ustc.edu.cn"
    ]
  ```

* 安装镜像
  
  * ubuntu:23.04

* 构建镜像
  
  ```
  root& docker build -f docker/Dockerfile -t ubuntu_cpp:v1 .
  ```

* 启动容器
  
  ```
  <!-- root& docker run -it -p 22:22 -p 80:80 -p 1234:1234 -p 5672:5672 -p 8080:8080 -p 15672:15672 -d -v Y:\Studyplace_Web_Development\Gateway_Control_System:/root/Gateway_Control_System --name control-env ubuntu_cpp:v1 -->
  root& docker run -it -P -d --network bridge -v Y:\Studyplace_Web_Development\Gateway_Control_System:/root/Gateway_Control_System --name control-env ubuntu_cpp:v1
  ```

* 配置完成，在本地编写代码，在容器中运行测试代码

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