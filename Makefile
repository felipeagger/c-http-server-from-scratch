CC = clang
CFLAGS = -static -lm -Wall -Wextra -O2

# files
SOURCES = simple-server.c epoll-server.c uring-server.c
TARGETS = simple epoll uring

all: $(TARGETS)

# Regras específicas para cada arquivo
simple: simple-server.c
	$(CC) -o simple-server simple-server.c request-handle.c cpu-bound.c $(CFLAGS)
#	$(CC) $(CFLAGS) -o $@ $< -DFILE1_DEFINE

epoll: epoll-server.c
	$(CC) -o epoll-server epoll-server.c request-handle.c cpu-bound.c $(CFLAGS)
#	$(CC) $(CFLAGS) -o $@ $< -DFILE2_DEFINE -DADDITIONAL_FLAG

uring: uring-server.c
	$(CC) -o uring-server uring-server.c request-handle.c cpu-bound.c $(CFLAGS) -I/usr/local/include -L/usr/local/lib -luring
#	# $(CC) $(CFLAGS) -g -o $@ $< -DFILE3_DEFINE -lm
#	# musl-gcc -o rest_server rest_server.c -I/usr/local/include -L/usr/local/lib -luring


clean:
	rm -f $(TARGETS)
