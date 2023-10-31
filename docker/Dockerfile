FROM ubuntu:23.04

RUN    apt-get update                       \
    && apt-get install -y apt-utils         \
    && apt-get install -y language-pack-en  \
    && apt-get install -y dialog            \
    && apt-get install -y vim               \
    && apt-get install -y net-tools         \
    && apt-get install -y openssh-server    \
    && apt-get install -y build-essential   \
    && apt-get install -y gdb               \
    && apt-get install -y git               \
    && apt-get install -y cmake             \
    && apt-get install -y libspdlog-dev     \
    && apt-get install -y libboost-all-dev  \
    && apt-get install -y libuv1-dev

RUN mkdir /root/Gateway_Control_System

EXPOSE 22 80 8080