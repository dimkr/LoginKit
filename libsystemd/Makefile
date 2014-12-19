include ../Makefile.common

LDFLAGS += -Wl,-soname=libsystemd.so.0 \
           -Wl,--no-as-needed \
           -L$(PWD)/../libsystemd-daemon \
           -L$(PWD)/../libsystemd-journal \
           -L$(PWD)/../libsystemd-login
LIBS += -lsystemd-daemon -lsystemd-journal -lsystemd-login

LIB = libsystemd.so

all: $(LIB)

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

$(LIB): $(OBJS)
	$(CC) -o $@ $(OBJS) -shared $(LDFLAGS) $(LIBS)

clean:
	rm -f $(LIB) $(OBJS)
