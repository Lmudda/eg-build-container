# Windows build environment for STM32 projects
FROM mcr.microsoft.com/windows/servercore:ltsc2022

# Install Chocolatey
RUN powershell -NoProfile -ExecutionPolicy Bypass -Command `
    "Set-ExecutionPolicy Bypass -Scope Process -Force; `
     [System.Net.ServicePointManager]::SecurityProtocol = `
     [System.Net.ServicePointManager]::SecurityProtocol -bor 3072; `
     iex ((New-Object System.Net.WebClient).DownloadString('https://chocolatey.org/install.ps1'))"

# Install required tools
RUN choco install -y git python cmake make

# Copy snapshots into the container
COPY snapshots C:\\opt\\snapshots

# Set default workdir
WORKDIR C:\\opt

# Default command
CMD ["cmd.exe"]
