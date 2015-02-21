include ../Makefile.common

PROG = loginkitd

all: $(PROG) org.freedesktop.login1.service

loginkitd-generated.c loginkitd-generated.h: interface.xml
	gdbus-codegen --generate-c-code \
	              loginkitd-generated \
	              --c-namespace LoginKit \
	              --interface-prefix org.freedesktop.login1. \
	              $^

%.o: %.c loginkitd-generated.h
	$(CC) -c -o $@ $< $(CFLAGS) $(GIO_CFLAGS)

$(PROG): $(OBJS) loginkitd-generated.o
	$(CC) -o $@ $^ $(LDFLAGS) $(LIBS) $(GIO_LIBS)

org.freedesktop.login1.service: org.freedesktop.login1.service.in
	sed s~"@SBIN_DIR@"~"$(SBIN_DIR)"~ $^ > $@

clean:
	rm -f org.freedesktop.login1.service $(PROG) $(OBJS) loginkitd-generated.h loginkitd-generated.c
