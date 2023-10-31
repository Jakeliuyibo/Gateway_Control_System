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
├─ CMakeLists.txt       
├─ docker               # docker配置文件
├─ logs                 # 日志文件
├─ README.md    
├─ src                  # 源文件
├─ test                 # 测试文件
└─ utility              # 组件
   ├─ logger            # 日志模块
   ├─ rabbitmqclient    # Rabbitmq模块
   ├─ reactor           # Reactor模块
   ├─ systime           # 系统时间模块
   └─ threadpool        # 线程池模块
```