FROM ubuntu:22.04

RUN apt-get update && \
    apt-get install -y wget gpg lsb-release && \
    rm -rf /var/lib/apt/lists/*

RUN wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | \
    gpg --dearmor - | \
    tee /usr/share/keyrings/kitware-archive-keyring.gpg >/dev/null && \
    echo "deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ jammy main" | \
    tee /etc/apt/sources.list.d/kitware.list >/dev/null

RUN apt-get update && \
    apt-get install -y build-essential cmake g++ && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .

RUN rm -rf build && \
    mkdir build && \
    cd build && \
    cmake .. && \
    make

CMD ["./build/untitled"]