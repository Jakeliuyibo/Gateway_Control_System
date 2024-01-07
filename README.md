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
  root& docker build -f docker/Dockerfile -t ubuntu_cpp:v2 .
  ```

* 启动容器
  
  ```
  root& docker run -it -P -d --network bridge -v [${PROJECT_DIR}]:/home/Gateway_Control_System --name control-env ubuntu_cpp:v2

  for example: root& docker run -it -P -d --network bridge -v Y:\Studyplace_Web_Development\Gateway_Control_System:/home/Gateway_Control_System -v Y:\Studyplace_Web_Development\Gateway_Management_System:/home/Gateway_Management_System --name gateway-env ubuntu_cpp:v2
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