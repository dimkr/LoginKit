include ../Makefile.common

CFLAGS += -pthread
LDFLAGS += -pthread \
           -Wl,-soname=libsystemd-login.so.0 \
           -Wl,--version-script=compat.sym

LIB = libsystemd-login.so

all: $(LIB) libsystemd-login.pc

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS) $(GIO_CFLAGS)

$(LIB): $(OBJS) compat.sym
	$(CC) -o $@ $(OBJS) -shared $(LDFLAGS) $(LIBS) $(GIO_LIBS)

libsystemd-login.pc: libsystemd-login.pc.in
	sed -e s~@PREFIX@~$(PREFIX)~g \
	    -e s~@LIB_DIR@~$(LIB_DIR)~g \
	    -e s~@INCLUDE_DIR@~$(INCLUDE_DIR)~g \
	    $^ > $@

clean:
	rm -f libsystemd-login.pc $(LIB) $(OBJS)
