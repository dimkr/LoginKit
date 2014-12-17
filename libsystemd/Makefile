include ../Makefile.common

LDFLAGS += -Wl,--version-script=compat.sym

LIB = libsystemd.so.0

all: $(LIB)

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS) $(GIO_CFLAGS)

$(LIB): $(OBJS) compat.sym
	$(CC) -o $@ $(OBJS) -shared $(LDFLAGS) $(LIBS) $(GIO_LIBS)

clean:
	rm -f $(LIB) $(OBJS)
