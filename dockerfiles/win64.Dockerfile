FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    mercurial \
    autoconf \
    libtool \
    bison \
    byacc \
    texinfo \
    g++-mingw-w64-x86-64-posix \
    gcc-mingw-w64-x86-64-posix \
    python3-pip
RUN pip3 install cmake

RUN hg clone --updaterev tip https://gmplib.org/repo/gmp-6.3 /gmp-source
RUN cd /gmp-source && \
    ./.bootstrap && \
    ./configure --host=x86_64-w64-mingw32 --enable-static --disable-shared --prefix=/usr/x86_64-w64-mingw32 && \
    make -j$(nproc) && \
    make install

WORKDIR /app

COPY . .

RUN cmake . --preset mingw && \
    cmake --build --preset mingw-build

RUN mkdir -p /build && \
    cp build_mingw/l2encdec.exe /build/l2encdec.exe

VOLUME /build

ENTRYPOINT ["/bin/sh", "-c", "\
    cp /build/l2encdec.exe /mounted-build/ && \
    chown $(stat -c '%u:%g' /mounted-build/) /mounted-build/l2encdec.exe \
    "]
