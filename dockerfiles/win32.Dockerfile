FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    mercurial \
    autoconf \
    libtool \
    bison \
    byacc \
    texinfo \
    g++-mingw-w64-i686-posix \
    gcc-mingw-w64-i686-posix \
    python3-pip
RUN pip3 install cmake

RUN hg clone --updaterev tip https://gmplib.org/repo/gmp-6.3 /gmp-source
RUN cd /gmp-source && \
    ./.bootstrap && \
    ./configure --host=i686-w64-mingw32 --enable-static --disable-shared --prefix=/usr/i686-w64-mingw32 && \
    make -j$(nproc) && \
    make install

WORKDIR /app

COPY . .

RUN cmake . --preset mingw-w64-i686 && \
    cmake --build --preset mingw-w64-i686-build

RUN mkdir -p /build && \
    cp build_mingw_i686/l2encdec.exe /build/l2encdec.exe

VOLUME /build

ENTRYPOINT ["/bin/sh", "-c", "\
    cp /build/l2encdec.exe /mounted-build/ && \
    chown $(stat -c '%u:%g' /mounted-build/) /mounted-build/l2encdec.exe \
    "]
