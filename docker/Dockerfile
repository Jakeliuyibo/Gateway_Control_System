FROM ubuntu:22.04

EXPOSE 22 80 1234 5672 6379 7100 7101 7205 7206 8080 8800 8801 15672 19999

ENV TZ=Asia/Shanghai
ENV TimeZone=Asia/Shanghai

# 安装基本工具
RUN    rm /etc/apt/sources.list                                                                                                           \
    && echo deb http://mirrors.tuna.tsinghua.edu.cn/ubuntu/ jammy main restricted universe multiverse           >> /etc/apt/sources.list  \
    && echo deb http://mirrors.tuna.tsinghua.edu.cn/ubuntu/ jammy-updates main restricted universe multiverse   >> /etc/apt/sources.list  \
    && echo deb http://mirrors.tuna.tsinghua.edu.cn/ubuntu/ jammy-backports main restricted universe multiverse >> /etc/apt/sources.list  \
    && echo deb http://security.ubuntu.com/ubuntu/ jammy-security main restricted universe multiverse           >> /etc/apt/sources.list  \
    && apt-get update                                                                                                                     \
    && apt-get install -y language-pack-en                                                                                                \
    && apt-get install -y apt-utils                                                                                                       \
    && apt-get install -y dialog                                                                                                          \
    && apt-get install -y vim                                                                                                             \
    && apt-get install -y systemctl                                                                                                       \
    && apt-get install -y net-tools                                                                                                       \
    && apt-get install -y iputils-ping                                                                                                    \
    && apt-get install -y build-essential                                                                                                 \
    && apt-get install -y gdb git cmake                                                                                                   \                                                                                         
    && apt-get install -y libssl-dev                                                                                                      \
    && apt-get install -y bsdmainutils                                                                                                    \                                                                                           
    && apt-get install -y ntp                                                                                                          

# 安装c++第三方库
RUN    apt-get install -y libspdlog-dev                                              \
    && apt-get install -y libboost-all-dev                                           \
    && apt-get install -y libuv1-dev                                                 \                           
    && apt-get install -y erlang rabbitmq-server                                     \
    && apt-get install -y libjsoncpp-dev

# 安装python和apache环境
RUN    apt-get install -y python3-dev python3-pip apache2 libapache2-mod-wsgi-py3    \
    && ln -s /usr/bin/python3 /usr/bin/python                                        \
    && apt-get install -y redis-server                                               \
    && pip install datedays==2.2.1                                                   \ 
    && pip install Flask==1.1.4                                                      \ 
    && pip install Flask_Session==0.4.0                                              \     
    && pip install Flask_SQLAlchemy==2.5.1                                           \         
    && pip install pika==1.3.1                                                       \         
    && pip install pyserial==3.5                                                     \ 
    && pip install SQLAlchemy==1.4.22                                                \  
    && pip install redis                                                             \     
    && pip install markupsafe==1.1.1                                                 \
    && pip install requests                                                                  

# 安装rabbitmq-server环境(TODO 开启自启动服务)
RUN    service rabbitmq-server start                                                 \
    && systemctl enable rabbitmq-server                                              \
    && rabbitmqctl add_user user 123456                                              \
    && rabbitmqctl set_user_tags user administrator                                  \
    && rabbitmqctl set_permissions -p / user ".*" ".*" ".*"                          \
    && service rabbitmq-server restart                                               \
    && rabbitmq-plugins enable rabbitmq_management                                   \
    && service rabbitmq-server restart                                                                                                          

# 安装rabbitmq-client环境
RUN cd /home                                                                         \
    && git clone https://github.com/alanxz/rabbitmq-c                                \
    && cd rabbitmq-c; mkdir build; cd build                                          \
    && cmake ..                                                                      \
    && make                                                                          \
    && make install

# 构建工程目录
RUN    cd /home                                                                      \
    && mkdir /home/Gateway_Control_System                                            \
    && mkdir /home/Gateway_Management_System
