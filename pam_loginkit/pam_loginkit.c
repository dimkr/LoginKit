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

#include <libsystemd-login/session.h>
#include <common/bus.h>

static gboolean close_session(GDBusConnection *bus, const char *cookie)
{
	GVariant *reply;
	GError *error = NULL;
	gboolean ret;

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
		g_log(G_LOG_DOMAIN,
		      G_LOG_LEVEL_ERROR,
		      "CloseSession() failed: %s",
		      error->message);
		g_error_free(error);
		goto end;
	}

	g_variant_get(reply, "(b)", &ret);
	g_variant_unref(reply);

end:
	return ret;
}

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
	int ret = PAM_SESSION_ERR;

	/* get the user name */
	if (PAM_SUCCESS != pam_get_user(pamh, &user, NULL))
		goto end;

	pam_syslog(pamh, LOG_DEBUG, "opening a session for %s", user);

	/* get the user ID */
	passwd = pam_modutil_getpwnam(pamh, user);
	if (NULL == passwd)
		goto end;

	/* connect to the bus */
	bus = bus_get();
	if (NULL == bus)
		goto end;

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
		g_log(G_LOG_DOMAIN,
		      G_LOG_LEVEL_ERROR,
		      "OpenSessionWithParameters() failed: %s",
		      error->message);
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
		g_log(G_LOG_DOMAIN,
		      G_LOG_LEVEL_ERROR,
		      "GetSessionForCookie() failed: %s",
		      error->message);
		g_error_free(error);
		goto close_session;
	}

	g_variant_get(reply, "(o)", &session);
	g_variant_unref(reply);

	/* store the ID in the XDG_SESSION_ID environment variable */
	if (PAM_SUCCESS != pam_misc_setenv(pamh, "XDG_SESSION_ID", session, 0))
		goto close_session;

	/* get the ID of the seat the session belongs to */
	if (0 != sd_session_get_seat(session, &seat))
		goto close_session;

	/* store the seat ID in the XDG_SEAT_ID environment variable */
	if (PAM_SUCCESS != pam_misc_setenv(pamh, "XDG_SEAT_ID", seat, 0))
		goto free_seat;

	/* get the session type */
	if (0 != sd_session_get_type(session, &type))
		goto free_seat;

	/* store the session type in the XDG_SESSION_TYPE environment variable */
	if (PAM_SUCCESS != pam_misc_setenv(pamh, "XDG_SESSION_TYPE", type, 0))
		goto free_type;

	/* create a temporary directory for the user under /run/user */
	len = snprintf(path,
	               sizeof(path),
	               "/run/user/%d",
	               (unsigned int) passwd->pw_uid);
	if ((0 >= len) || (sizeof(path) <= len))
		goto free_type;

	if (-1 == mkdir(path, 0644)) {
		if (EEXIST != errno)
			goto free_type;
	}

	if (-1 == chown(path, passwd->pw_uid, passwd->pw_gid))
		goto free_type;

	/* store the directory path in XDG_RUNTIME_DIR environment variable */
	if (PAM_SUCCESS == pam_misc_setenv(pamh, "XDG_RUNTIME_DIR", path, 0))
		ret = PAM_SUCCESS;

free_type:
	free(type);

free_seat:
	free(seat);

close_session:
	/* upon failure, close the ConsoleKit session */
	if (PAM_SUCCESS != ret)
		(void) close_session(bus, cookie);

close_bus:
	/* disconnect from the bus */
	bus_close();

end:
	return ret;
}

__attribute__((visibility("default")))
int pam_sm_close_session(pam_handle_t *pamh,
                         int flags,
                         int argc,
                         const char **argv)
{
	GDBusConnection *bus;
	const char *cookie;
	gboolean closed;
	int ret = PAM_SESSION_ERR;

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
	closed = close_session(bus, cookie);
	if (TRUE == closed)
		ret = PAM_SUCCESS;

	/* disconnect from the bus */
	bus_close();

end:
	return ret;
}
