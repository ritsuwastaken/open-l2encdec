FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    git \
    g++-mingw-w64-i686-posix \
    gcc-mingw-w64-i686-posix \
    python3-pip
RUN pip3 install cmake

WORKDIR /app

COPY . .

RUN cd examples/cli && \
    cmake . --preset mingw-w64-i686 && \
    cmake --build --preset mingw-w64-i686-build

RUN mkdir -p /build && \
    cp examples/cli/build_mingw_i686/l2encdec_win.exe /build/l2encdec.exe

VOLUME /build

ENTRYPOINT ["/bin/sh", "-c", "\
    cp /build/l2encdec.exe /mounted-build/ && \
    chown $(stat -c '%u:%g' /mounted-build/) /mounted-build/l2encdec.exe \
    "]
