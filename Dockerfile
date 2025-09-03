# Use a Linux base image for the build environment
FROM ubuntu:22.04

# Set environment variables to avoid prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Update package lists and install necessary build tools
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    git \
    unzip \
    && rm -rf /var/lib/apt/lists/*

# Create a build directory
WORKDIR /usr/src/app

# Clean up
RUN apt-get autoremove -y && apt-get clean && rm -rf /var/lib/apt/lists/*

# Default command
CMD ["bash"]
