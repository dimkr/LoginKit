include ../Makefile.common

LIB = libloginkit-common.a

all: $(LIB)

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS) $(GIO_CFLAGS)

$(LIB): $(OBJS)
	ar rcs $@ $^

clean:
	rm -f $(LIB) $(OBJS)
