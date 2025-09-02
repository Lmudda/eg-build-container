# Environment for building STM32 projects under Ubuntu Linux
# Copyright (c) eg technology Ltd, 2024-2025. All rights reserved.

# Base image (a LTS version)
FROM ubuntu:24.04 AS base

# Install pre-requisites.
RUN apt-get update && \ 
    apt-get install -y \
        git=1:2.43.0-1ubuntu7.2 \
        python3=3.12.3-0ubuntu2 \
        python3-pip=24.0+dfsg-1ubuntu1.1 \
		build-essential=12.10ubuntu1 \
		gcc-multilib=4:13.2.0-7ubuntu1 \
		g++-multilib=4:13.2.0-7ubuntu1

# Copy the STM32CubeCLT installer file. By default this is local and a specific version.
ARG CUBECLT_URL="./st-stm32cubeclt_1.18.0_24403_20250225_1636_amd64.sh"
ARG CUBECLT_VER="1.18.0"
ADD ${CUBECLT_URL} /tmp/stm32cubeclt-install.sh

# Install STM32CubeCLT.
# Need to set LICENSE_ALREADY_ACCEPTED=1 to avoid having to confirm license interactively.
RUN chmod +x /tmp/stm32cubeclt-install.sh && \
    LICENSE_ALREADY_ACCEPTED=1 /tmp/stm32cubeclt-install.sh /opt/st/stm32cubeclt_${CUBECLT_VER} && \
    rm /tmp/stm32cubeclt-install.sh

# Setup the path to include the ST versions of CMake and the GNU toolchain
ENV PATH="${PATH}:/opt/st/stm32cubeclt_${CUBECLT_VER}/CMake/bin:/opt/st/stm32cubeclt_${CUBECLT_VER}/GNU-tools-for-STM32/bin"

# Provide a shell
CMD ["/bin/bash"]
