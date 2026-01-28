## 基础镜像，与你之前的 gcc/ubuntu 版本保持一致
#FROM ubuntu:20.04
#
## 设置非交互模式
#ENV DEBIAN_FRONTEND=noninteractive
#
#RUN sed -i 's/ports.ubuntu.com/mirrors.tuna.tsinghua.edu.cn/g' /etc/apt/sources.list
#
## 更新软件源并安装 QEMU 8.0 编译所需的所有核心依赖
#RUN apt-get update && apt-get install -y --no-install-recommends \
#    # 1. 基础构建工具
#    build-essential \
#    python3 \
#    python3-pip \
#    python3-setuptools \
#    ninja-build \
#    pkg-config \
#    git \
#    wget \
#    \
#    # 2. QEMU 核心依赖
#    libglib2.0-dev \
#    libpixman-1-dev \
#    \
#    # 3. 图形界面与显示支持
#    libgtk-3-dev \
#    libsdl2-dev \
#    libepoxy-dev \
#    libdrm-dev \
#    libgbm-dev \
#    \
#    # 4. 网络与用户模式支持
#    libslirp-dev \
#    libvdeplug-dev \
#    # [新增] 虚拟文件系统支持 (virtfs)
#    libcap-ng-dev \
#    libattr1-dev \
#    \
#    # 5. 加密库与安全 (修正了 nettle-dev 的包名)
#    libgnutls28-dev \
#    libgcrypt20-dev \
#    nettle-dev \
#    \
#    # 6. 高级 I/O 与存储特性 (移除了 Ubuntu 20.04 不支持的 liburing-dev)
#    libaio-dev \
#    zlib1g-dev \
#    libzstd-dev \
#    libbz2-dev \
#    \
#    # 7. 音频支持
#    libasound2-dev \
#    libpulse-dev \
#    \
#    # 清理 apt 缓存
#    && apt-get clean \
#    && rm -rf /var/lib/apt/lists/*
#
## 设置默认工作目录
#WORKDIR /workspace
#
## 默认启动 bash
#CMD ["/bin/bash"]

#FROM ubuntu:20.04
#
#ENV DEBIAN_FRONTEND=noninteractive
#
#RUN sed -i 's/ports.ubuntu.com/mirrors.tuna.tsinghua.edu.cn/g' /etc/apt/sources.list
#
#RUN apt-get update && apt-get install -y --no-install-recommends \
#    build-essential \
#    python3 python3-pip python3-setuptools \
#    ninja-build pkg-config git wget \
#    \
#    libglib2.0-dev libpixman-1-dev \
#    libgtk-3-dev libsdl2-dev libepoxy-dev libdrm-dev libgbm-dev \
#    libslirp-dev libvdeplug-dev libcap-ng-dev libattr1-dev \
#    libgnutls28-dev libgcrypt20-dev nettle-dev \
#    libaio-dev zlib1g-dev libzstd-dev libbz2-dev \
#    libasound2-dev libpulse-dev \
#    \
#    #

FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive

RUN sed -i 's/ports.ubuntu.com/mirrors.tuna.tsinghua.edu.cn/g' /etc/apt/sources.list

# 确保 universe 可用（交叉工具链在 universe 里）
RUN apt-get update && apt-get install -y --no-install-recommends \
    software-properties-common \
    && add-apt-repository universe \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

RUN apt-get update && apt-get install -y --no-install-recommends \
    # 0. 常用基础
    ca-certificates \
    tzdata \
    xz-utils \
    \
    # 1. 基础构建工具
    build-essential \
    python3 python3-pip python3-setuptools \
    ninja-build pkg-config git wget \
    \
    # 2. QEMU 核心依赖
    libglib2.0-dev \
    libpixman-1-dev \
    \
    # 3. 图形界面与显示支持
    libgtk-3-dev \
    libsdl2-dev \
    libepoxy-dev \
    libdrm-dev \
    libgbm-dev \
    \
    # 4. 网络与用户模式支持
    libslirp-dev \
    libvdeplug-dev \
    libcap-ng-dev \
    libattr1-dev \
    \
    # 5. 加密库与安全
    libgnutls28-dev \
    libgcrypt20-dev \
    nettle-dev \
    \
    # 6. 高级 I/O 与存储特性
    libaio-dev \
    zlib1g-dev \
    libzstd-dev \
    libbz2-dev \
    \
    # 7. 音频支持
    libasound2-dev \
    libpulse-dev \
    \
    # 8. RISC-V bare-metal 工具链 + dtc + newlib
    device-tree-compiler \
    gcc-riscv64-unknown-elf \
    binutils-riscv64-unknown-elf \
    libnewlib-dev \
    gdb-multiarch \
    \
    # 构建时验收
    && riscv64-unknown-elf-gcc --version \
    && dtc --version \
    \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# 可选安装：libnewlib-nano（focal 可能没有该包名，有就装，没有就跳过）
RUN apt-get update \
    && (apt-get install -y --no-install-recommends libnewlib-nano || true) \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /workspace
CMD ["/bin/bash"]


