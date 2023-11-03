FROM ubuntu:22.04

EXPOSE 22 80 1234 5672 8080 15672

# 安装基本工具
RUN    rm /etc/apt/sources.list                                                                                                                 \
    && echo deb http://mirrors.tuna.tsinghua.edu.cn/ubuntu/ jammy main restricted universe multiverse              >> /etc/apt/sources.list     \
    && echo deb http://mirrors.tuna.tsinghua.edu.cn/ubuntu/ jammy-updates main restricted universe multiverse      >> /etc/apt/sources.list     \
    && echo deb http://mirrors.tuna.tsinghua.edu.cn/ubuntu/ jammy-backports main restricted universe multiverse    >> /etc/apt/sources.list     \
    && echo deb http://security.ubuntu.com/ubuntu/ jammy-security main restricted universe multiverse               >> /etc/apt/sources.list    \
    && apt-get update                                                                                                                           \
    && apt-get install -y language-pack-en                                                                                                      \
    && apt-get install -y apt-utils                                                                                                             \
    && apt-get install -y dialog                                                                                                                \
    && apt-get install -y vim                                                                                                                   \
    && apt-get install -y systemctl                                                                                                             \
    && apt-get install -y net-tools                                                                                                             \
    && apt-get install -y iputils-ping                                                                                                          \
    && apt-get install -y build-essential                                                                                                       \
    && apt-get install -y gdb git cmake                                                                                                         \                                                                                         
    && apt-get install -y libssl-dev

# 安装c++第三方库
RUN    apt-get install -y libspdlog-dev                                                                                                         \
    && apt-get install -y libboost-all-dev                                                                                                      \
    && apt-get install -y libuv1-dev                                                                                                            \                           
    && apt-get install -y erlang rabbitmq-server                                                                                                \
    && apt-get install -y libjsoncpp-dev

# 安装rabbitmq-server环境(TODO 开启自启动服务)
RUN    service rabbitmq-server start                                                                                                            \
    && systemctl enable rabbitmq-server                                                                                                         \
    && rabbitmqctl add_user user 123456                                                                                                         \
    && rabbitmqctl set_user_tags user administrator                                                                                             \
    && rabbitmqctl set_permissions -p / user ".*" ".*" ".*"                                                                                     \
    && service rabbitmq-server restart                                                                                                          \
    && rabbitmq-plugins enable rabbitmq_management                                                                                              \
    && service rabbitmq-server restart                                                                                                          

# 安装rabbitmq-client环境
RUN cd /root                                                                                                                                    \
    && git clone https://github.com/alanxz/rabbitmq-c                                                                                           \
    && cd rabbitmq-c; mkdir build; cd build                                                                                                     \
    && cmake ..                                                                                                                                 \
    && make                                                                                                                                     \
    && make install

# 构建工程目录
RUN    cd /root                                                                                                                                 \
    && mkdir /root/Gateway_Control_System