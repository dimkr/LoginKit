include ../Makefile.common

LDFLAGS += -Wl,-soname=libloginkit-daemon.so.0 -Wl,--version-script=compat.sym

LIB = libloginkit-daemon.so

all: $(LIB)

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS) $(GLIB_CFLAGS)

$(LIB): $(OBJS) compat.sym
	$(CC) -o $@ $(OBJS) -shared $(LDFLAGS) $(LIBS) $(GLIB_LIBS)

clean:
	rm -f $(LIB) $(OBJS)
