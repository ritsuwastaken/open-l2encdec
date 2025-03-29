FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    git \
    g++-mingw-w64-i686-posix \
    gcc-mingw-w64-i686-posix \
    python3-pip
RUN pip3 install cmake

RUN git clone --depth 1 -b openssl-3.4.1 https://github.com/openssl/openssl.git
RUN cd openssl && \
    ./Configure mingw no-tests \
        --static \
        --prefix=/usr/i686-w64-mingw32 \
        --cross-compile-prefix=i686-w64-mingw32- && \
    make -j$(nproc) && \
    make install_sw

WORKDIR /app

COPY . .

RUN cmake . --preset mingw-w64-i686 \
        -DOPENSSL_ROOT_DIR=/usr/i686-w64-mingw32 \
        -DBUILD_SHARED_LIBS=OFF && \
    cmake --build --preset mingw-w64-i686-build

RUN mkdir -p /build && \
    cp build_mingw_i686/l2encdec.exe /build/l2encdec.exe

VOLUME /build

ENTRYPOINT ["/bin/sh", "-c", "\
    cp /build/l2encdec.exe /mounted-build/ && \
    chown $(stat -c '%u:%g' /mounted-build/) /mounted-build/l2encdec.exe \
    "]
