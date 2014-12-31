include ./Makefile.common

DESTDIR ?=

INSTALL = install -v
LN = ln -v

all: libloginkit-daemon/libloginkit-daemon.so \
     libloginkit-journal/libloginkit-journal.so \
     libloginkit-login/libloginkit-login.so \
     libloginkit/libloginkit.so \
     loginkitd/loginkitd \
     pam_loginkit/pam_loginkit.so

common/libloginkit-common.a:
	cd common; $(MAKE)

libloginkit-daemon/libloginkit-daemon.so: common/libloginkit-common.a
	cd libloginkit-daemon; $(MAKE)

libloginkit-journal/libloginkit-journal.so: common/libloginkit-common.a
	cd libloginkit-journal; $(MAKE)

loginkitd/loginkitd: common/libloginkit-common.a
	cd loginkitd; $(MAKE)

libloginkit-login/libloginkit-login.so: common/libloginkit-common.a
	cd libloginkit-login; $(MAKE)

libloginkit/libloginkit.so: libloginkit-daemon/libloginkit-daemon.so \
                          libloginkit-journal/libloginkit-journal.so \
                          libloginkit-login/libloginkit-login.so
	cd libloginkit; $(MAKE)

pam_loginkit/pam_loginkit.so: common/libloginkit-common.a
	cd pam_loginkit; $(MAKE)

install: all
	$(INSTALL) -D -m 644 libloginkit-daemon/libloginkit-daemon.so $(DESTDIR)/$(LIB_DIR)/libloginkit-daemon.so.0.1
	$(LN) -s libloginkit-daemon.so.0.1 $(DESTDIR)/$(LIB_DIR)/libloginkit-daemon.so.0
	$(LN) -s libloginkit-daemon.so.0.1 $(DESTDIR)/$(LIB_DIR)/libloginkit-daemon.so
	$(INSTALL) -D -m 644 libloginkit-daemon/misc.h $(DESTDIR)/$(INCLUDE_DIR)/libloginkit-daemon/misc.h
	$(INSTALL) -m 644 libloginkit-daemon/fd.h $(DESTDIR)/$(INCLUDE_DIR)/libloginkit-daemon/fd.h
	$(INSTALL) -D -m 644 libloginkit-daemon/sd-daemon.h $(DESTDIR)/$(INCLUDE_DIR)/systemd/sd-daemon.h

	$(INSTALL) -m 644 libloginkit-journal/libloginkit-journal.so $(DESTDIR)/$(LIB_DIR)/libloginkit-journal.so.0.1
	$(LN) -s libloginkit-journal.so.0.1 $(DESTDIR)/$(LIB_DIR)/libloginkit-journal.so.0
	$(LN) -s libloginkit-journal.so.0.1 $(DESTDIR)/$(LIB_DIR)/libloginkit-journal.so
	$(INSTALL) -D -m 644 libloginkit-journal/journal.h $(DESTDIR)/$(INCLUDE_DIR)/libloginkit-journal/journal.h
	$(LN) -s ../libloginkit-journal/journal.h $(DESTDIR)/$(INCLUDE_DIR)/systemd/sd-journal.h

	$(INSTALL) -m 644 libloginkit-login/libloginkit-login.so $(DESTDIR)/$(LIB_DIR)/libloginkit-login.so.0.1
	$(LN) -s libloginkit-login.so.0.1 $(DESTDIR)/$(LIB_DIR)/libloginkit-login.so.0
	$(LN) -s libloginkit-login.so.0.1 $(DESTDIR)/$(LIB_DIR)/libloginkit-login.so
	$(INSTALL) -D -m 644 libloginkit-login/session.h $(DESTDIR)/$(INCLUDE_DIR)/libloginkit-login/session.h
	$(INSTALL) -m 644 libloginkit-login/seat.h $(DESTDIR)/$(INCLUDE_DIR)/libloginkit-login/seat.h
	$(INSTALL) -m 644 libloginkit-login/pid.h $(DESTDIR)/$(INCLUDE_DIR)/libloginkit-login/pid.h
	$(INSTALL) -m 644 libloginkit-login/uid.h $(DESTDIR)/$(INCLUDE_DIR)/libloginkit-login/uid.h
	$(INSTALL) -m 644 libloginkit-login/monitor.h $(DESTDIR)/$(INCLUDE_DIR)/libloginkit-login/monitor.h
	$(INSTALL) -m 644 libloginkit-login/sd-login.h $(DESTDIR)/$(INCLUDE_DIR)/systemd/sd-login.h

	$(INSTALL) -m 644 libloginkit/libloginkit.so $(DESTDIR)/$(LIB_DIR)/libloginkit.so.0.1
	$(LN) -s libloginkit.so.0.1 $(DESTDIR)/$(LIB_DIR)/libloginkit.so.0
	$(LN) -s libloginkit.so.0.1 $(DESTDIR)/$(LIB_DIR)/libloginkit.so
	$(INSTALL) -D -m 644 libloginkit/libloginkit.pc $(DESTDIR)/$(LIB_DIR)/pkgconfig/libloginkit.pc

	$(INSTALL) -D -m 644 pam_loginkit/pam_loginkit.so $(DESTDIR)/$(LIB_DIR)/security/pam_loginkit.so
	$(INSTALL) -D -m 755 loginkitd/loginkitd $(DESTDIR)/$(SBIN_DIR)/loginkitd

	$(LN) -s libloginkit-daemon.so.0.1 $(DESTDIR)/$(LIB_DIR)/libsystemd-daemon.so.0
	$(LN) -s libloginkit-journal.so.0.1 $(DESTDIR)/$(LIB_DIR)/libsystemd-journal.so.0
	$(LN) -s libloginkit-login.so.0.1 $(DESTDIR)/$(LIB_DIR)/libsystemd-login.so.0
	$(LN) -s libloginkit.so.0.1 $(DESTDIR)/$(LIB_DIR)/libsystemd.so.0

	$(LN) -s libloginkit.pc $(DESTDIR)/$(LIB_DIR)/pkgconfig/libloginkit-daemon.pc
	$(LN) -s libloginkit.pc $(DESTDIR)/$(LIB_DIR)/pkgconfig/libloginkit-journal.pc
	$(LN) -s libloginkit.pc $(DESTDIR)/$(LIB_DIR)/pkgconfig/libloginkit-login.pc
	$(LN) -s libloginkit.pc $(DESTDIR)/$(LIB_DIR)/pkgconfig/libsystemd-daemon.pc
	$(LN) -s libloginkit.pc $(DESTDIR)/$(LIB_DIR)/pkgconfig/libsystemd-journal.pc
	$(LN) -s libloginkit.pc $(DESTDIR)/$(LIB_DIR)/pkgconfig/libsystemd-login.pc

clean:
	cd pam_loginkit; $(MAKE) clean
	cd loginkitd; $(MAKE) clean
	cd libloginkit; $(MAKE) clean
	cd libloginkit-login; $(MAKE) clean
	cd libloginkit-journal; $(MAKE) clean
	cd libloginkit-daemon; $(MAKE) clean
	cd common; $(MAKE) clean
