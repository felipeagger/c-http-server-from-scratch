# c-http-server-from-scratch

This is a implementation of Http server using directly Linux SystemCalls.

There are some implementations:
- simple-server.c: Using only accept syscall.
- epoll-server.c: Using epoll syscall with 10 events.
- io_uring.c: TODO.

### Default port is 8080.

# Endpoints:
- GET /health-check: just return Healthy!
- GET /api/io-bound: simulate io-bound with sleep
- GET /api/cpu-bound: simulate cpu-bound with prime numbers

# Params QueryString
- timeout: in ms (default is 10ms)

# Entrypoints:
- /home/simple-server
- /home/epoll-server