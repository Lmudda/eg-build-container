# Use a Linux base image for the build environment
FROM ubuntu:22.04

# Set environment variables to avoid prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Update package lists and install necessary build tools, including Java
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    git \
    unzip \
    openjdk-17-jdk \
    && rm -rf /var/lib/apt/lists/*

# Set the working directory
WORKDIR /usr/src/app

# Copy the snapshot source files into the container image
# The snapshot files must be in the same directory as this Dockerfile
COPY snapshots/Submodules\ -\ 20-Aug-2025.zip /tmp/snapshots/
COPY snapshots/eg_otway_stm32\ -\ 20-Aug-2025.zip /tmp/snapshots/

# Clean up
RUN apt-get autoremove -y && apt-get clean && rm -rf /var/lib/apt/lists/*

# Default command
CMD ["bash"]