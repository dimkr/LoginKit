DESTDIR ?=
PREFIX ?= /usr

SBIN_DIR = $(PREFIX)/sbin
DOC_DIR = $(PREFIX)/share/doc
LIB_DIR = $(PREFIX)/lib
INCLUDE_DIR = $(PREFIX)/include

INSTALL = install -v
LN = ln -v

all: libsystemd/libsystemd.so.0 loginkitd/loginkitd pam_loginkit/pam_loginkit.so

common/libloginkit-common.a:
	cd common; $(MAKE)

libsystemd/libsystemd.so.0: common/libloginkit-common.a
	cd libsystemd; $(MAKE)

loginkitd/loginkitd: common/libloginkit-common.a
	cd loginkitd; $(MAKE)

pam_loginkit/pam_loginkit.so: common/libloginkit-common.a
	cd pam_loginkit; $(MAKE)

install: all
	$(INSTALL) -D -m 644 pam_loginkit/pam_loginkit.so $(DESTDIR)/$(LIB_DIR)/security/pam_loginkit.so
	$(INSTALL) -D -m 644 libsystemd/libsystemd.so.0 $(DESTDIR)/$(LIB_DIR)/libsystemd.so.0.0.1
	$(LN) -s libsystemd.so.0.0.1 $(DESTDIR)/$(LIB_DIR)/libsystemd.so.0
	$(INSTALL) -D -m 755 loginkitd/loginkitd $(DESTDIR)/$(SBIN_DIR)/loginkitd

	$(INSTALL) -D -m 644 libsystemd/session.h $(DESTDIR)/$(INCLUDE_DIR)/systemd/session.h
	$(INSTALL) -D -m 644 libsystemd/seat.h $(DESTDIR)/$(INCLUDE_DIR)/systemd/seat.h
	$(INSTALL) -D -m 644 libsystemd/pid.h $(DESTDIR)/$(INCLUDE_DIR)/systemd/pid.h
	$(INSTALL) -D -m 644 libsystemd/sd-login.h $(DESTDIR)/$(INCLUDE_DIR)/systemd/sd-login.h
	$(INSTALL) -D -m 644 libsystemd/journal.h $(DESTDIR)/$(INCLUDE_DIR)/systemd/sd-journal.h
	$(INSTALL) -D -m 644 libsystemd/sd-daemon.h $(DESTDIR)/$(INCLUDE_DIR)/systemd/sd-daemon.h

clean:
	cd pam_loginkit; $(MAKE) clean
	cd loginkitd; $(MAKE) clean
	cd libsystemd; $(MAKE) clean
	cd common; $(MAKE) clean
