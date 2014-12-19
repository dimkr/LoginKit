include ../Makefile.common

LDFLAGS += -Wl,-soname=libsystemd.so.0 -Wl,--version-script=compat.sym

OBJS = $(wildcard ../libsystemd-daemon/*.o) \
       $(wildcard ../libsystemd-journal/*.o) \
       $(wildcard ../libsystemd-login/*.o)

LIB = libsystemd.so

all: $(LIB)

$(LIB): $(OBJS)
	$(CC) -o $@ $(OBJS) -shared $(LDFLAGS) $(LIBS) $(GIO_LIBS)

clean:
	rm -f $(LIB) $(OBJS)
