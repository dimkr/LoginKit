include ../Makefile.common

LIB = pam_loginkit.so

all: $(LIB)

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS) $(GIO_CFLAGS)

$(LIB): $(OBJS)
	$(CC) -o $@ $^ -shared $(LDFLAGS) $(LIBS) $(GIO_LIBS) $(PAM_LIBS)

clean:
	rm -f $(LIB) $(OBJS)
