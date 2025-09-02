# Start from Jenkins agent base (includes Java, JNLP, SSH support)
FROM jenkins/agent:latest-jdk11

# Install required tools (cmake, gcc-arm, unzip, etc.)
USER root
RUN apt-get update && apt-get install -y \
    build-essential cmake git unzip wget \
    gcc-arm-none-eabi binutils-arm-none-eabi \
    && rm -rf /var/lib/apt/lists/*

# Copy snapshot zips into container
COPY snapshots/ /opt/snapshots/

# Set working directory
WORKDIR /workspace

# Default command (keeps agent alive)
ENTRYPOINT ["jenkins-agent"]
