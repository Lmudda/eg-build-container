# Use Windows Server Core as base
FROM mcr.microsoft.com/windows/servercore:ltsc2022

# Install Chocolatey
RUN powershell -NoProfile -ExecutionPolicy Bypass -Command `
    Set-ExecutionPolicy Bypass -Scope Process -Force; `
    [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072; `
    iex ((New-Object System.Net.WebClient).DownloadString('https://chocolatey.org/install.ps1'))

# Install tools
RUN choco install -y git python cmake make

# Copy snapshots (Windows path must use / forward slashes or escaped backslashes)
COPY snapshots C:/opt/snapshots

# Default command
CMD ["cmd.exe"]
