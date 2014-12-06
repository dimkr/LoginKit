CC ?= cc
CFLAGS ?= -O2 -pipe
LDFLAGS ?= -s
PKG_CONFIG ?= pkg-config

GIO_CFLAGS = $(shell $(PKG_CONFIG) --cflags gio-2.0 gio-unix-2.0)
GIO_LIBS = $(shell $(PKG_CONFIG) --libs gio-2.0 gio-unix-2.0)

PAM_LIBS = -lpam

SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)

CFLAGS += -std=gnu99 \
          -Wall \
          -pedantic \
          -pthread \
          -fvisibility=hidden \
          -fPIC \
          -DG_LOG_DOMAIN=\"LoginKit\"
LDFLAGS += -pthread -fPIC

LIB = libsystemd-login.so.0
PROG = loginkitd
PAM_MOD = pam_loginkit.so

all: $(LIB) $(PROG) $(PAM_MOD)

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS) $(GIO_CFLAGS)

loginkitd-generated.c: interface.xml
	gdbus-codegen --generate-c-code \
	              loginkitd-generated \
	              --c-namespace LoginKit \
	              --interface-prefix org.freedesktop.login1. \
	              $^

$(PROG): bus.o loginkitd-generated.o seat.o session.o power.o loginkitd.o
	$(CC) -o $@ $^ $(LDFLAGS) $(GIO_LIBS)

$(LIB): bus.o loginkit.o compat.sym
	$(CC) -o $@ bus.o loginkit.o -shared $(LDFLAGS) $(GIO_LIBS) -Wl,--version-script=compat.sym

$(PAM_MOD): bus.o pam_loginkit.o
	$(CC) -o $@ $^ -shared $(LDFLAGS) $(GIO_LIBS) $(PAM_LIBS)

clean:
	rm -f $(PAM_MOD) $(PROG) loginkitd-generated.h loginkitd-generated.c $(LIB) $(OBJS)
