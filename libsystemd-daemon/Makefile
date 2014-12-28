include ../Makefile.common

LDFLAGS += -Wl,-soname=libsystemd-daemon.so.0 -Wl,--version-script=compat.sym

LIB = libsystemd-daemon.so

all: $(LIB) libsystemd-daemon.pc

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS) $(GLIB_CFLAGS)

$(LIB): $(OBJS) compat.sym
	$(CC) -o $@ $(OBJS) -shared $(LDFLAGS) $(LIBS) $(GLIB_LIBS)

libsystemd-daemon.pc: libsystemd-daemon.pc.in
	sed -e s~@PREFIX@~$(PREFIX)~g \
	    -e s~@LIB_DIR@~$(LIB_DIR)~g \
	    -e s~@INCLUDE_DIR@~$(INCLUDE_DIR)~g \
	    $^ > $@

clean:
	rm -f libsystemd-daemon.pc $(LIB) $(OBJS)
