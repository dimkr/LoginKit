include ../Makefile.common

LDFLAGS += -Wl,-soname=libsystemd-journal.so.0 -Wl,--version-script=compat.sym

LIB = libsystemd-journal.so

all: $(LIB) libsystemd-journal.pc

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

$(LIB): $(OBJS) compat.sym
	$(CC) -o $@ $(OBJS) -shared $(LDFLAGS) $(LIBS)

libsystemd-journal.pc: libsystemd-journal.pc.in
	sed -e s~@PREFIX@~$(PREFIX)~g \
	    -e s~@LIB_DIR@~$(LIB_DIR)~g \
	    -e s~@INCLUDE_DIR@~$(INCLUDE_DIR)~g \
	    $^ > $@

clean:
	rm -f libsystemd-journal.pc $(LIB) $(OBJS)
