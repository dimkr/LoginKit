/*
 * this file is part of LoginKit.
 *
 * Copyright (c) 2014 Dima Krasner
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <syslog.h>
#include <stdio.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#define PAM_SM_SESSION
#include <security/pam_modules.h>
#include <security/pam_misc.h>
#include <security/pam_ext.h>

#include "bus.h"

__attribute__((visibility("default"))) PAM_EXTERN
int pam_sm_open_session(pam_handle_t *pamh,
                        int flags,
                        int argc,
                        const char **argv)
{
	char path[PATH_MAX];
	GDBusConnection *bus;
	GVariant *reply;
	GError *error = NULL;
	char *session;
	char *seat;
	char *type;
	char *cookie;
	uid_t owner;
	int len;

	pam_syslog(pamh, LOG_DEBUG, "opening a session for %ld", (long) getpid());

	bus = bus_get();
	if (NULL == bus)
		return PAM_SESSION_ERR;

	reply = g_dbus_connection_call_sync(bus,
	                                    "org.freedesktop.ConsoleKit",
	                                    "/org/freedesktop/ConsoleKit/Manager",
	                                    "org.freedesktop.ConsoleKit.Manager",
	                                    "OpenSession",
	                                    NULL,
	                                    G_VARIANT_TYPE("(s)"),
	                                    G_DBUS_CALL_FLAGS_NONE,
	                                    -1,
	                                    NULL,
	                                    &error);
	if (NULL == reply) {
		if (NULL != error)
			g_error_free(error);
		goto close_bus;
	}

	g_variant_get(reply, "(s)", &cookie);
	g_variant_unref(reply);

	/* TODO: close the session if setenv() fails */
	if (PAM_SUCCESS != pam_misc_setenv(pamh, "XDG_SESSION_COOKIE", cookie, 0))
		goto close_bus;

	reply = g_dbus_connection_call_sync(bus,
	                                    "org.freedesktop.ConsoleKit",
	                                    "/org/freedesktop/ConsoleKit/Manager",
	                                    "org.freedesktop.ConsoleKit.Manager",
	                                    "GetSessionForCookie",
	                                    g_variant_new("(s)", cookie),
	                                    G_VARIANT_TYPE("(o)"),
	                                    G_DBUS_CALL_FLAGS_NONE,
	                                    -1,
	                                    NULL,
	                                    &error);
	if (NULL == reply) {
		if (NULL != error)
			g_error_free(error);
		goto close_bus;
	}

	g_variant_get(reply, "(o)", &session);
	g_variant_unref(reply);

	if (PAM_SUCCESS != pam_misc_setenv(pamh, "XDG_SESSION_ID", session, 0))
		goto close_bus;

	reply = g_dbus_connection_call_sync(bus,
	                                    "org.freedesktop.ConsoleKit",
	                                    session,
	                                    "org.freedesktop.ConsoleKit.Session",
	                                    "GetSeatId",
	                                    NULL,
	                                    G_VARIANT_TYPE("(o)"),
	                                    G_DBUS_CALL_FLAGS_NONE,
	                                    -1,
	                                    NULL,
	                                    &error);
	if (NULL == reply) {
		if (NULL != error)
			g_error_free(error);
		goto close_bus;
	}

	g_variant_get(reply, "(o)", &seat);
	g_variant_unref(reply);

	if (PAM_SUCCESS != pam_misc_setenv(pamh, "XDG_SEAT_ID", seat, 0))
		goto close_bus;

	reply = g_dbus_connection_call_sync(bus,
	                                    "org.freedesktop.ConsoleKit",
	                                    session,
	                                    "org.freedesktop.ConsoleKit.Session",
	                                    "GetSessionType",
	                                    NULL,
	                                    G_VARIANT_TYPE("(s)"),
	                                    G_DBUS_CALL_FLAGS_NONE,
	                                    -1,
	                                    NULL,
	                                    &error);
	if (NULL == reply) {
		if (NULL != error)
			g_error_free(error);
		goto close_bus;
	}

	g_variant_get(reply, "(s)", &type);
	g_variant_unref(reply);

	if (PAM_SUCCESS != pam_misc_setenv(pamh, "XDG_SESSION_TYPE", type, 0))
		goto close_bus;

	reply = g_dbus_connection_call_sync(bus,
	                                    "org.freedesktop.ConsoleKit",
	                                    session,
	                                    "org.freedesktop.ConsoleKit.Session",
	                                    "GetUnixUser",
	                                    NULL,
	                                    G_VARIANT_TYPE("(u)"),
	                                    G_DBUS_CALL_FLAGS_NONE,
	                                    -1,
	                                    NULL,
	                                    &error);
	if (NULL == reply) {
		if (NULL != error)
			g_error_free(error);
		goto close_bus;
	}

	g_variant_get(reply, "(u)", &owner);
	g_variant_unref(reply);

	len = snprintf(path, sizeof(path), "/run/user/%d", (unsigned int) owner);
	if ((0 >= len) || (sizeof(path) <= len))
		goto close_bus;

	if (-1 == mkdir(path, 0644)) {
		if (EEXIST != errno)
			goto close_bus;
	}

	if (PAM_SUCCESS != pam_misc_setenv(pamh, "XDG_RUNTIME_DIR", path, 0))
		goto close_bus;

	return PAM_SUCCESS;

close_bus:
	bus_close();
	return PAM_SESSION_ERR;
}

__attribute__((visibility("default"))) PAM_EXTERN
int pam_sm_close_session(pam_handle_t *pamh,
                         int flags,
                         int argc,
                         const char **argv)
{
	GDBusConnection *bus;
	GVariant *reply;
	GError *error = NULL;
	const char *cookie;
	gboolean closed;
	int ret;

	pam_syslog(pamh, LOG_DEBUG, "closing a session for %ld", (long) getpid());

	cookie = pam_getenv(pamh, "XDG_SESSION_COOKIE");
	if (NULL == cookie)
		goto end;

	bus = bus_get();
	if (NULL == bus)
		goto end;

	reply = g_dbus_connection_call_sync(bus,
	                                    "org.freedesktop.ConsoleKit",
	                                    "/org/freedesktop/ConsoleKit/Manager",
	                                    "org.freedesktop.ConsoleKit.Manager",
	                                    "CloseSession",
	                                    g_variant_new("(s)", cookie),
	                                    G_VARIANT_TYPE("(b)"),
	                                    G_DBUS_CALL_FLAGS_NONE,
	                                    -1,
	                                    NULL,
	                                    &error);
	if (NULL == reply) {
		if (NULL != error)
			g_error_free(error);
		goto close_bus;
	}

	g_variant_get(reply, "(b)", &closed);
	g_variant_unref(reply);

	if (FALSE == closed)
		goto close_bus;

	ret = PAM_SUCCESS;

close_bus:
	bus_close();

end:
	return ret;
}
