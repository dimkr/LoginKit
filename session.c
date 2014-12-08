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

#include <glib/gstdio.h>

#include "bus.h"
#include "session.h"

static char *get_session_id(GDBusConnection *bus, const char *session)
{
	GVariant *reply;
	GError *error = NULL;
	char *id;

	reply = g_dbus_connection_call_sync(bus,
	                                    "org.freedesktop.ConsoleKit",
	                                    session,
	                                    "org.freedesktop.ConsoleKit.Session",
	                                    "GetId",
	                                    NULL,
	                                    G_VARIANT_TYPE("(o)"),
	                                    G_DBUS_CALL_FLAGS_NONE,
	                                    -1,
	                                    NULL,
	                                    &error);
	if (NULL == reply) {
		if (NULL != error)
			g_error_free(error);
		return NULL;
	}

	g_variant_get(reply, "(o)", &id);
	g_variant_unref(reply);

	return id;
}

static char *get_session_by_id(GDBusConnection *bus, const char *id)
{
	GVariantIter iter;
	GVariant *reply;
	GError *error = NULL;
	GVariant *sessions;
	char *session;
	char *ret = NULL;
	char *session_id;

	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_INFO, "getting the session of %s", id);

	reply = g_dbus_connection_call_sync(bus,
	                                    "org.freedesktop.ConsoleKit",
	                                    "org/freedesktop/ConsoleKit/Manager",
	                                    "org.freedesktop.ConsoleKit.Manager",
	                                    "GetSessions",
	                                    NULL,
	                                    G_VARIANT_TYPE("(ao)"),
	                                    G_DBUS_CALL_FLAGS_NONE,
	                                    -1,
	                                    NULL,
	                                    &error);
	if (NULL == reply) {
		if (NULL != error)
			g_error_free(error);
		return NULL;
	}

	sessions = g_variant_get_child_value(reply, 0);
	g_variant_iter_init(&iter, sessions);

	while (TRUE == g_variant_iter_loop(&iter, "o", &session, NULL)) {
		session_id = get_session_id(bus, session);
		if (NULL == session_id)
			continue;

		if (0 == g_strcmp0(id, session_id)) {
			ret = session;
			break;
		}
	}

	g_variant_unref(reply);
	g_variant_unref(sessions);

	return ret;
}

gboolean on_handle_unlock_session(LoginKitManager *interface,
                                  GDBusMethodInvocation *invocation,
                                  const gchar *id,
                                  gpointer user_data)
{
	GDBusConnection *bus;
	GVariant *reply;
	GError *error = NULL;
	char *session;
	gboolean ret = FALSE;

	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_INFO, "unlocking %s", id);

	bus = bus_get();
	if (NULL == bus)
		goto end;

	/* find the session with the passed ID */
	session = get_session_by_id(bus, id);
	if (NULL == session)
		goto end;

	reply = g_dbus_connection_call_sync(bus,
	                                    "org.freedesktop.ConsoleKit",
	                                    session,
	                                    "org.freedesktop.ConsoleKit.Session",
	                                    "Unlock",
	                                    NULL,
	                                    NULL,
	                                    G_DBUS_CALL_FLAGS_NONE,
	                                    -1,
	                                    NULL,
	                                    &error);
	if (NULL == reply) {
		if (NULL != error)
			g_error_free(error);
		goto end;
	}

	g_variant_unref(reply);

	ret = TRUE;

end:
	login_kit_manager_complete_unlock_session(interface, invocation);
	return ret;
}

gboolean on_handle_activate_session_on_seat(LoginKitManager *interface,
                                            GDBusMethodInvocation *invocation,
                                            const gchar *arg_session,
                                            const gchar *arg_seat,
                                            gpointer user_data)
{
	GDBusConnection *bus;
	GVariant *reply;
	GError *error = NULL;
	char *session;
	gboolean ret = FALSE;

	g_log(G_LOG_DOMAIN,
	      G_LOG_LEVEL_INFO,
	      "activating %s on %s",
	      arg_session,
	      arg_seat);

	bus = bus_get();
	if (NULL == bus)
		goto end;

	/* find the session with the passed ID */
	session = get_session_by_id(bus, arg_session);
	if (NULL == session)
		goto end;

	reply = g_dbus_connection_call_sync(bus,
	                                    "org.freedesktop.ConsoleKit",
	                                    arg_session,
	                                    "org.freedesktop.ConsoleKit.Session",
	                                    "Activate",
	                                    NULL,
	                                    NULL,
	                                    G_DBUS_CALL_FLAGS_NONE,
	                                    -1,
	                                    NULL,
	                                    &error);
	if (NULL == reply) {
		if (NULL != error)
			g_error_free(error);
		goto end;
	}

	g_variant_unref(reply);

	ret = TRUE;

end:
	login_kit_manager_complete_activate_session_on_seat(interface, invocation);
	return ret;
}

gboolean on_handle_get_session(LoginKitManager *interface,
                               GDBusMethodInvocation *invocation,
                               const gchar *arg_ssid,
                               gpointer user_data)
{
	login_kit_manager_complete_get_session(interface, invocation, arg_ssid);
}
