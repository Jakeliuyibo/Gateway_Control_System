# Gateway_Control_System

# 安装和运行
## software
* vscode(cmake\git\c++\gdb\Remote Development等插件)
* git
* docker
## docker
* 安装hyper-v、wsl2、docker desktop
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
  root& docker run -it -p 22:22 -p 80:80 -p 8080:8080 -d -v Y:\Studyplace_Web_Development\Gateway_Control_System:/root/Gateway_Control_System --name control-env ubuntu_cpp:v1
  ```
* 使用vscode attach到容器中进行开发（在容器中vscode服务器安装相应插件）