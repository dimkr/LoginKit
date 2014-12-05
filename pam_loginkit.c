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

#define PAM_SM_SESSION
#include <security/pam_modules.h>
#include <security/pam_misc.h>

#include "bus.h"

__attribute__((visibility("default"))) PAM_EXTERN
int pam_sm_open_session(pam_handle_t *pamh,
                        int flags,
                        int argc,
                        const char **argv)
{
	GDBusConnection *bus;
	GVariant *reply;
	GError *error = NULL;
	char *id;
	char *cookie;

	g_log(G_LOG_DOMAIN,
	      G_LOG_LEVEL_INFO,
	      "opening a session for %ld",
	      (long) getpid());

	/* TODO: call bus_close() with atexit() or something */
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
		return PAM_SESSION_ERR;
	}

	g_variant_get(reply, "(s)", &cookie);
	g_variant_unref(reply);

	/* TODO: close the session if setenv() fails */
	if (PAM_SUCCESS != pam_misc_setenv(pamh, "XDG_SESSION_COOKIE", cookie, 0))
		return PAM_SESSION_ERR;

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
		return PAM_SESSION_ERR;
	}

	g_variant_get(reply, "(o)", &id);
	g_variant_unref(reply);

	if (PAM_SUCCESS != pam_misc_setenv(pamh, "XDG_SESSION_ID", id, 0))
		return PAM_SESSION_ERR;

	return PAM_SUCCESS;
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
	gboolean ret;

	g_log(G_LOG_DOMAIN,
	      G_LOG_LEVEL_INFO,
	      "closing a session for %ld",
	      (long) getpid());

	cookie = pam_getenv(pamh, "XDG_SESSION_COOKIE");
	if (NULL == cookie)
		return PAM_SESSION_ERR;

	bus = bus_get();
	if (NULL == bus)
		return PAM_SESSION_ERR;

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
		return PAM_SESSION_ERR;
	}

	g_variant_get(reply, "(b)", &ret);
	g_variant_unref(reply);

	if (FALSE == ret)
		return PAM_SESSION_ERR;

	return PAM_SUCCESS;
}
