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
    build-essential \
    libgmp-dev \
    cmake

RUN hg clone --updaterev tip https://gmplib.org/repo/gmp /gmp-source
RUN cd /gmp-source && \
    ./.bootstrap && \
    ./configure --host=x86_64-w64-mingw32 --enable-static --disable-shared --prefix=/usr/x86_64-w64-mingw32 && \
    make -j$(nproc) && \
    make install

WORKDIR /app

COPY . .

RUN cmake . -B 'docker_build_linux' && \
    cmake --build 'docker_build_linux'

RUN cmake . -B 'docker_build_windows' \
    -DCMAKE_TOOLCHAIN_FILE=cmake/mingw-w64-x86_64.cmake \
    -DCMAKE_BUILD_TYPE=Release && \
    cmake --build 'docker_build_windows'

RUN mkdir -p /build && \
    cp docker_build_linux/l2encdec /build/l2encdec && \
    cp docker_build_windows/l2encdec.exe /build/

VOLUME /build

ENTRYPOINT ["/bin/sh", "-c", "cp /build/l2encdec /mounted-build/ && cp /build/l2encdec.exe /mounted-build/ && chown $(stat -c '%u:%g' /mounted-build/) /mounted-build/l2encdec && chown $(stat -c '%u:%g' /mounted-build/) /mounted-build/l2encdec.exe"]
