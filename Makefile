include ./Makefile.common

DESTDIR ?=

INSTALL = install -v
LN = ln -v

all: loginkitd/loginkitd

loginkitd/loginkitd:
	cd loginkitd; $(MAKE)

install: all
	$(INSTALL) -D -m 755 loginkitd/loginkitd $(DESTDIR)/$(SBIN_DIR)/loginkitd
	$(INSTALL) -D -m 644 loginkitd/org.freedesktop.login1.service $(DESTDIR)/$(DATA_DIR)/dbus-1/system-services/org.freedesktop.login1.service

clean:
	cd loginkitd; $(MAKE) clean
