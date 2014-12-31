include ../Makefile.common

LDFLAGS += -Wl,-soname=libloginkit.so.0 -Wl,--version-script=compat.sym

OBJS = $(wildcard ../libloginkit-daemon/*.o) \
       $(wildcard ../libloginkit-journal/*.o) \
       $(wildcard ../libloginkit-login/*.o)

LIB = libloginkit.so

all: $(LIB) libloginkit.pc

$(LIB): $(OBJS)
	$(CC) -o $@ $(OBJS) -shared $(LDFLAGS) $(LIBS) $(GIO_LIBS)

libloginkit.pc: libloginkit.pc.in
	sed -e s~@PREFIX@~$(PREFIX)~g \
	    -e s~@LIB_DIR@~$(LIB_DIR)~g \
	    -e s~@INCLUDE_DIR@~$(INCLUDE_DIR)~g \
	    $^ > $@

clean:
	rm -f libloginkit.pc $(LIB) $(OBJS)
