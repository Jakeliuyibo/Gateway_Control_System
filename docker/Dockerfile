FROM ubuntu:23.04

RUN    apt-get update                       \
    && apt-get install -y vim               \
    && apt-get install -y net-tools         \
    && apt-get install -y openssh-server    \
    && apt-get install -y build-essential   \
    && apt-get install -y gdb               \
    && apt-get install -y git

RUN mkdir /root/Gateway_Control_System

EXPOSE 22 80 8080