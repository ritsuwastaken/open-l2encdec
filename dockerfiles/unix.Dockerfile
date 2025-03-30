FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    git \
    build-essential \
    python3-pip

RUN pip3 install cmake

WORKDIR /app

COPY . .

RUN cmake . --preset unix && \
    cmake --build --preset unix-build

RUN mkdir -p /build && \
    cp build_unix/l2encdec /build/l2encdec

VOLUME /build

ENTRYPOINT ["/bin/sh", "-c", "\
    cp /build/l2encdec /mounted-build/ && \
    chown $(stat -c '%u:%g' /mounted-build/) /mounted-build/l2encdec \
    "]
