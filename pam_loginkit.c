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
#include <security/pam_modutil.h>

#include "bus.h"

__attribute__((visibility("default")))
int pam_sm_open_session(pam_handle_t *pamh,
                        int flags,
                        int argc,
                        const char **argv)
{
	char path[PATH_MAX];
	GVariantBuilder builder;
	GDBusConnection *bus;
	GVariant *reply;
	GError *error = NULL;
	char *session;
	char *seat;
	char *type;
	char *cookie;
	const char *user;
	struct passwd *passwd;
	int len;

	/* get the user name */
	if (PAM_SUCCESS != pam_get_user(pamh, &user, NULL))
		goto failure;

	pam_syslog(pamh, LOG_DEBUG, "opening a session for %s", user);

	/* get the user ID */
	passwd = pam_modutil_getpwnam(pamh, user);
	if (NULL == passwd)
		goto failure;

	/* connect to the bus */
	bus = bus_get();
	if (NULL == bus)
		goto failure;

	/* start a ConsoleKit session for the user and and obtain a session
	 * cookie */
	g_variant_builder_init(&builder, G_VARIANT_TYPE("a(sv)"));
	g_variant_builder_add(&builder,
	                      "(sv)",
	                      "unix-user",
	                      g_variant_new_int32(passwd->pw_uid));
	reply = g_dbus_connection_call_sync(bus,
	                                    "org.freedesktop.ConsoleKit",
	                                    "/org/freedesktop/ConsoleKit/Manager",
	                                    "org.freedesktop.ConsoleKit.Manager",
	                                    "OpenSessionWithParameters",
	                                    g_variant_new("(a(sv))", &builder),
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

	/* store the cookie in the XDG_SESSION_COOKIE environment variable */
	if (PAM_SUCCESS != pam_misc_setenv(pamh, "XDG_SESSION_COOKIE", cookie, 0))
		goto close_session;

	/* translate the session cookie to the session ID */
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
		goto close_session;
	}

	g_variant_get(reply, "(o)", &session);
	g_variant_unref(reply);

	/* store the ID in the XDG_SESSION_ID environment variable */
	if (PAM_SUCCESS != pam_misc_setenv(pamh, "XDG_SESSION_ID", session, 0))
		goto close_session;

	/* get the ID of the seat the session belongs to */
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
		goto close_session;
	}

	g_variant_get(reply, "(o)", &seat);
	g_variant_unref(reply);

	/* store the seat ID in the XDG_SEAT_ID environment variable */
	if (PAM_SUCCESS != pam_misc_setenv(pamh, "XDG_SEAT_ID", seat, 0))
		goto close_session;

	/* get the session type */
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
		goto close_session;
	}

	g_variant_get(reply, "(s)", &type);
	g_variant_unref(reply);

	/* store the session type in the XDG_SESSION_TYPE environment variable */
	if (PAM_SUCCESS != pam_misc_setenv(pamh, "XDG_SESSION_TYPE", type, 0))
		goto close_session;

	/* create a temporary directory for the user under /run/user */
	len = snprintf(path,
	               sizeof(path),
	               "/run/user/%d",
	               (unsigned int) passwd->pw_uid);
	if ((0 >= len) || (sizeof(path) <= len))
		goto close_session;

	if (-1 == mkdir(path, 0644)) {
		if (EEXIST != errno)
			goto close_session;
	}

	/* store the directory path in XDG_RUNTIME_DIR environment variable */
	if (PAM_SUCCESS == pam_misc_setenv(pamh, "XDG_RUNTIME_DIR", path, 0))
		return PAM_SUCCESS;

close_session:
	/* upon failure, close the ConsoleKit session */
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
	}

	g_variant_unref(reply);

close_bus:
	/* disconnect from the bus */
	bus_close();

failure:
	return PAM_SESSION_ERR;
}

__attribute__((visibility("default")))
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

	/* get the session cookie from the XDG_SESSION_COOKIE environment
	 * variable created by pam_sm_open_session() */
	cookie = pam_getenv(pamh, "XDG_SESSION_COOKIE");
	if (NULL == cookie)
		goto end;

	/* connect to the bus */
	bus = bus_get();
	if (NULL == bus)
		goto end;

	/* close the ConsoleKit session */
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
	/* disconnect from the bus */
	bus_close();

end:
	return ret;
}
