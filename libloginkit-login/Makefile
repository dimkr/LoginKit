include ../Makefile.common

CFLAGS += -pthread
LDFLAGS += -pthread \
           -Wl,-soname=libloginkit-login.so.0 \
           -Wl,--version-script=compat.sym

LIB = libloginkit-login.so

all: $(LIB)

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS) $(GIO_CFLAGS)

$(LIB): $(OBJS) compat.sym
	$(CC) -o $@ $(OBJS) -shared $(LDFLAGS) $(LIBS) $(GIO_LIBS)

clean:
	rm -f $(LIB) $(OBJS)
