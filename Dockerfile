# Use Windows Server Core as base
FROM mcr.microsoft.com/windows/servercore:ltsc2022

# Install Chocolatey
RUN powershell -NoProfile -ExecutionPolicy Bypass -Command "Set-ExecutionPolicy Bypass -Scope Process -Force; [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072; iex ((New-Object System.Net.WebClient).DownloadString('https://chocolatey.org/install.ps1'))"

# Install required tools including Java (needed for Jenkins agent)
RUN choco install -y git python cmake make openjdk17

# Set JAVA_HOME and update PATH
ENV JAVA_HOME="C:\\Program Files\\OpenJDK\\jdk-17"
ENV PATH="%JAVA_HOME%\\bin;%PATH%"

# Copy snapshots into container
COPY snapshots C:/opt/snapshots

# Default shell
CMD ["cmd.exe"]
