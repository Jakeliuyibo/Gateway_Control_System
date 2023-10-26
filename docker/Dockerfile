FROM ubuntu:23.04

RUN    apt-get update                       \
    && apt-get install apt-utils            \
    && apt-get install language-pack-en     \
    && apt-get install dialog               \
    && apt-get install -y vim               \
    && apt-get install -y net-tools         \
    && apt-get install -y openssh-server    \
    && apt-get install -y build-essential   \
    && apt-get install -y gdb               \
    && apt-get install -y git               \
    && apt-get install -y cmake             \
    && apt-get install libspdlog-dev        \
    && apt-get install libjsoncpp-dev       \
    && apt-get install libsimpleini-dev     \
    && apt-get install libconfig++-dev      \
    && apt-get install libboost-all-dev     \

RUN mkdir /root/Gateway_Control_System

EXPOSE 22 80 8080