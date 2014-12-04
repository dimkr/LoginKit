CC ?= cc
CFLAGS ?= -O2 -pipe
LDFLAGS ?= -s
PKG_CONFIG ?= pkg-config

GIO_CFLAGS = $(shell $(PKG_CONFIG) --cflags gio-2.0)
GIO_LIBS = $(shell $(PKG_CONFIG) --libs gio-2.0)

SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)

CFLAGS += -std=gnu99 -Wall -pedantic -fvisibility=hidden -fPIC
LDFLAGS += -shared -fPIC

LIB = libsystemd-login.so.0

all: $(LIB)

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS) $(GIO_CFLAGS)

$(LIB): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS) $(GIO_LIBS)

clean:
	rm -f $(LIB) $(OBJS)
