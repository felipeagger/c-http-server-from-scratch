FROM alpine:latest AS build

WORKDIR /app

RUN apk add --no-cache \
    clang \
    llvm \
    musl-dev \
    build-base \
    git \
    linux-headers
    #liburing-dev


# liburing
RUN git clone https://github.com/axboe/liburing.git && \
    cd liburing && \
    make && \
    make install
    #ldconfig

COPY . .

# Compile - # clang main.c -o app -static
RUN make all

# Etapa final: contêiner mínimo para executar o binário
FROM alpine:latest

WORKDIR /home

COPY --from=build /app/simple-server .
COPY --from=build /app/epoll-server .
COPY --from=build /app/uring-server .

