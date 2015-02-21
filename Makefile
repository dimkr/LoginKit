include ./Makefile.common

DESTDIR ?=

INSTALL = install -v
LN = ln -v

all: loginkitd/loginkitd

loginkitd/loginkitd:
	cd loginkitd; $(MAKE)

install: all
	$(INSTALL) -D -m 755 loginkitd/loginkitd $(DESTDIR)/$(SBIN_DIR)/loginkitd

clean:
	cd loginkitd; $(MAKE) clean
