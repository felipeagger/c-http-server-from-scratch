FROM alpine:latest AS build

WORKDIR /app

RUN apk add --no-cache \
    clang \
    llvm \
    musl-dev \
    build-base

COPY . .

# Compile - # clang main.c -o app -static
RUN make all

# Etapa final: contêiner mínimo para executar o binário
FROM alpine:latest

WORKDIR /home

COPY --from=build /app/simple-server .
COPY --from=build /app/epoll-server .

#CMD ["./app"]
