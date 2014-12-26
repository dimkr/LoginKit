include ./Makefile.common

DESTDIR ?=

INSTALL = install -v
LN = ln -v

all: libsystemd-daemon/libsystemd-daemon.so \
     libsystemd-journal/libsystemd-journal.so \
     libsystemd-login/libsystemd-login.so \
     libsystemd/libsystemd.so \
     loginkitd/loginkitd \
     pam_loginkit/pam_loginkit.so

common/libloginkit-common.a:
	cd common; $(MAKE)

libsystemd-daemon/libsystemd-daemon.so: common/libloginkit-common.a
	cd libsystemd-daemon; $(MAKE)

libsystemd-journal/libsystemd-journal.so: common/libloginkit-common.a
	cd libsystemd-journal; $(MAKE)

loginkitd/loginkitd: common/libloginkit-common.a
	cd loginkitd; $(MAKE)

libsystemd-login/libsystemd-login.so: common/libloginkit-common.a
	cd libsystemd-login; $(MAKE)

libsystemd/libsystemd.so: libsystemd-daemon/libsystemd-daemon.so \
                          libsystemd-journal/libsystemd-journal.so \
                          libsystemd-login/libsystemd-login.so
	cd libsystemd; $(MAKE)

pam_loginkit/pam_loginkit.so: common/libloginkit-common.a
	cd pam_loginkit; $(MAKE)

install: all
	$(INSTALL) -D -m 644 libsystemd-daemon/libsystemd-daemon.so $(DESTDIR)/$(LIB_DIR)/libsystemd-daemon.so.0.1
	$(LN) -s libsystemd-daemon.so.0.1 $(DESTDIR)/$(LIB_DIR)/libsystemd-daemon.so.0
	$(LN) -s libsystemd-daemon.so.0.1 $(DESTDIR)/$(LIB_DIR)/libsystemd-daemon.so
	$(INSTALL) -D -m 644 libsystemd-daemon/misc.h $(DESTDIR)/$(INCLUDE_DIR)/systemd/misc.h
	$(INSTALL) -D -m 644 libsystemd-daemon/fd.h $(DESTDIR)/$(INCLUDE_DIR)/systemd/fd.h
	$(INSTALL) -D -m 644 libsystemd-daemon/sd-daemon.h $(DESTDIR)/$(INCLUDE_DIR)/systemd/sd-daemon.h
	$(INSTALL) -D -m 644 libsystemd-daemon/libsystemd-daemon.pc $(DESTDIR)/$(LIB_DIR)/pkgconfig/libsystemd-daemon.pc

	$(INSTALL) -m 644 libsystemd-journal/libsystemd-journal.so $(DESTDIR)/$(LIB_DIR)/libsystemd-journal.so.0.1
	$(LN) -s libsystemd-journal.so.0.1 $(DESTDIR)/$(LIB_DIR)/libsystemd-journal.so.0
	$(LN) -s libsystemd-journal.so.0.1 $(DESTDIR)/$(LIB_DIR)/libsystemd-journal.so
	$(INSTALL) -m 644 libsystemd-journal/sd-journal.h $(DESTDIR)/$(INCLUDE_DIR)/systemd/sd-journal.h
	$(INSTALL) -m 644 libsystemd-journal/libsystemd-journal.pc $(DESTDIR)/$(LIB_DIR)/pkgconfig/libsystemd-journal.pc

	$(INSTALL) -m 644 libsystemd-login/libsystemd-login.so $(DESTDIR)/$(LIB_DIR)/libsystemd-login.so.0.1
	$(LN) -s libsystemd-login.so.0.1 $(DESTDIR)/$(LIB_DIR)/libsystemd-login.so.0
	$(LN) -s libsystemd-login.so.0.1 $(DESTDIR)/$(LIB_DIR)/libsystemd-login.so
	$(INSTALL) -m 644 libsystemd-login/session.h $(DESTDIR)/$(INCLUDE_DIR)/systemd/session.h
	$(INSTALL) -m 644 libsystemd-login/seat.h $(DESTDIR)/$(INCLUDE_DIR)/systemd/seat.h
	$(INSTALL) -m 644 libsystemd-login/pid.h $(DESTDIR)/$(INCLUDE_DIR)/systemd/pid.h
	$(INSTALL) -m 644 libsystemd-login/uid.h $(DESTDIR)/$(INCLUDE_DIR)/systemd/uid.h
	$(INSTALL) -m 644 libsystemd-login/monitor.h $(DESTDIR)/$(INCLUDE_DIR)/systemd/monitor.h
	$(INSTALL) -m 644 libsystemd-login/sd-login.h $(DESTDIR)/$(INCLUDE_DIR)/systemd/sd-login.h
	$(INSTALL) -m 644 libsystemd-login/libsystemd-login.pc $(DESTDIR)/$(LIB_DIR)/pkgconfig/libsystemd-login.pc

	$(INSTALL) -m 644 libsystemd/libsystemd.so $(DESTDIR)/$(LIB_DIR)/libsystemd.so.0.1
	$(LN) -s libsystemd.so.0.1 $(DESTDIR)/$(LIB_DIR)/libsystemd.so.0
	$(LN) -s libsystemd.so.0.1 $(DESTDIR)/$(LIB_DIR)/libsystemd.so

	$(INSTALL) -D -m 644 pam_loginkit/pam_loginkit.so $(DESTDIR)/$(LIB_DIR)/security/pam_loginkit.so
	$(INSTALL) -D -m 755 loginkitd/loginkitd $(DESTDIR)/$(SBIN_DIR)/loginkitd

clean:
	cd pam_loginkit; $(MAKE) clean
	cd loginkitd; $(MAKE) clean
	cd libsystemd; $(MAKE) clean
	cd libsystemd-login; $(MAKE) clean
	cd libsystemd-journal; $(MAKE) clean
	cd libsystemd-daemon; $(MAKE) clean
	cd common; $(MAKE) clean
