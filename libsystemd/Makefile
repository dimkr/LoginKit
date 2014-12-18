include ../Makefile.common

LDFLAGS += -Wl,--version-script=compat.sym

LIB = libsystemd.so.0

all: $(LIB) libsystemd.pc

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS) $(GIO_CFLAGS)

$(LIB): $(OBJS) compat.sym
	$(CC) -o $@ $(OBJS) -shared $(LDFLAGS) $(LIBS) $(GIO_LIBS)

libsystemd.pc: libsystemd.pc.in
	sed -e s~@PREFIX@~$(PREFIX)~g \
	    -e s~@LIB_DIR@~$(LIB_DIR)~g \
	    -e s~@INCLUDE_DIR@~$(INCLUDE_DIR)~g \
	    $^ > $@

clean:
	rm -f libsystemd.pc $(LIB) $(OBJS)
