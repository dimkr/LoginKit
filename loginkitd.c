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

#include <errno.h>

#include <glib/gstdio.h>

#include "bus.h"
#include "loginkitd-generated.h"

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

static gboolean on_handle_unlock_session(LoginKitManager *interface,
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

static char *get_seat_id(GDBusConnection *bus, const char *seat)
{
	GVariant *reply;
	GError *error = NULL;
	char *id;

	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_INFO, "getting the ID of %s", seat);

	reply = g_dbus_connection_call_sync(bus,
	                                    "org.freedesktop.ConsoleKit",
	                                    seat,
	                                    "org.freedesktop.ConsoleKit.Seat",
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

/* the logind documentation says: "ListSeats() returns an array with all
 * currently available seats. The structure in the array consists of the
 * following fields: seat id, seat object path.". however, ConsoleKit's
 * GetSeats method returns only the object path, so we have to call
 * GetId for each seat */
static gboolean on_handle_list_seats(LoginKitManager *interface,
                                     GDBusMethodInvocation *invocation,
                                     gpointer user_data)
{
	GVariantIter iter;
	GVariant *reply;
	GError *error = NULL;
	GDBusConnection *bus;
	GVariantBuilder *builder;
	GVariant *ret;
	GVariant *seats;
	char *seat;
	char *id;

	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_INFO, "listing available seats");

	bus = bus_get();
	if (NULL == bus)
		return FALSE;

	/* list all seats */
	reply = g_dbus_connection_call_sync(bus,
	                                    "org.freedesktop.ConsoleKit",
	                                    "/org/freedesktop/ConsoleKit/Manager",
	                                    "org.freedesktop.ConsoleKit.Manager",
	                                    "GetSeats",
	                                    NULL,
	                                    G_VARIANT_TYPE("(ao)"),
	                                    G_DBUS_CALL_FLAGS_NONE,
	                                    -1,
	                                    NULL,
	                                    &error);
	if (NULL == reply) {
		if (NULL != error)
			g_error_free(error);
		return FALSE;
	}

	seats = g_variant_get_child_value(reply, 0);
	g_variant_iter_init(&iter, seats);

	/* create a builder object, to initialize an array */
	builder = g_variant_builder_new(G_VARIANT_TYPE("a(so)"));

	/* do a best-effort attempt to enumerate all seats */
	while (TRUE == g_variant_iter_loop(&iter, "o", &seat, NULL)) {
		id = get_seat_id(bus, seat);
		if (NULL == id)
			continue;
		g_variant_builder_add(builder, "(so)", id, seat);
	}

	g_variant_unref(reply);
	g_variant_unref(seats);

	ret = g_variant_new("a(so)", builder);
	g_variant_builder_unref(builder);

	login_kit_manager_complete_list_seats(interface, invocation, ret);

	return TRUE;
}

static gboolean on_handle_activate_session_on_seat(
                                              LoginKitManager *interface,
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

static void on_seat_added(GDBusConnection *connection,
                          const gchar *sender_name,
                          const gchar *object_path,
                          const gchar *interface_name,
                          const gchar *signal_name,
                          GVariant *parameters,
                          gpointer user_data)
{
	GDBusConnection *bus;
	char *seat;
	char *id;

	bus = bus_get();
	if (NULL == bus)
		return;

	g_variant_get(parameters, "(o)", &seat);
	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_INFO, "a seat has been added: %s", seat);

	id = get_seat_id(bus, seat);
	if (NULL == id)
		return;

	login_kit_manager_emit_seat_new((LoginKitManager *) user_data, id, seat);
}

static void on_seat_removed(GDBusConnection *connection,
                            const gchar *sender_name,
                            const gchar *object_path,
                            const gchar *interface_name,
                            const gchar *signal_name,
                            GVariant *parameters,
                            gpointer user_data)
{
	GDBusConnection *bus;
	char *seat;
	char *id;

	bus = bus_get();
	if (NULL == bus)
		return;

	g_variant_get(parameters, "(o)", &seat);
	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_INFO, "a seat has been removed: %s", seat);

	id = get_seat_id(bus, seat);
	if (NULL == id)
		return;

	login_kit_manager_emit_seat_removed((LoginKitManager *) user_data,
	                                    id,
	                                    seat);
}

static guint g_added_signal = 0;
static guint g_removed_signal = 0;

static void signals_subscribe(LoginKitManager *interface)
{
	GDBusConnection *bus;

	bus = bus_get();
	if (NULL == bus)
		return;

	g_added_signal = g_dbus_connection_signal_subscribe(
	                                       bus,
	                                       "org.freedesktop.ConsoleKit",
	                                       "org.freedesktop.ConsoleKit.Manager",
	                                       "SeatAdded",
	                                       "/org/freedesktop/Consolekit",
	                                       NULL,
	                                       G_DBUS_SIGNAL_FLAGS_NONE,
	                                       on_seat_added,
	                                       interface,
	                                       NULL);

	g_removed_signal = g_dbus_connection_signal_subscribe(
	                                       bus,
	                                       "org.freedesktop.ConsoleKit",
	                                       "org.freedesktop.ConsoleKit.Manager",
	                                       "SeatRemoved",
	                                       "/org/freedesktop/Consolekit",
	                                       NULL,
	                                       G_DBUS_SIGNAL_FLAGS_NONE,
	                                       on_seat_removed,
	                                       interface,
	                                       NULL);
}

static void on_bus_acquired(GDBusConnection *bus,
                            const gchar *name,
                            gpointer user_data)
{
	LoginKitManager *interface;
	GError *error = NULL;

	interface = login_kit_manager_skeleton_new();
	g_signal_connect(interface,
	                 "handle-unlock-session",
	                 G_CALLBACK(on_handle_unlock_session),
	                 NULL);
	g_signal_connect(interface,
	                 "handle-list-seats",
	                 G_CALLBACK(on_handle_list_seats),
	                 NULL);
	g_signal_connect(interface,
	                 "handle-activate-session-on-seat",
	                 G_CALLBACK(on_handle_activate_session_on_seat),
	                 NULL);

	signals_subscribe(interface);

	if (!g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(interface),
	                                      bus,
	                                      "/org/freedesktop/login1",
	                                      &error)) {
		if (NULL != error)
			g_error_free(error);
	}
}


static void on_name_acquired(GDBusConnection *bus,
                             const gchar *name,
                             gpointer user_data)
{
}

static void on_name_lost(GDBusConnection *bus,
                         const gchar *name,
                         gpointer user_data)
{
	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "lost the bus name");
	g_main_loop_quit((GMainLoop *) user_data);
}

static void signals_unsubscribe(void)
{
	GDBusConnection *bus;

	bus = bus_get();
	if (NULL == bus)
		return;

	if (0 != g_added_signal) {
		g_dbus_connection_signal_unsubscribe(bus, g_added_signal);
		g_added_signal = 0;
	}

	if (0 != g_removed_signal) {
		g_dbus_connection_signal_unsubscribe(bus, g_removed_signal);
		g_removed_signal = 0;
	}
}

int main(int argc, char *argv[])
{
	GMainLoop *loop;
	guint bus;

	loop = g_main_loop_new(NULL, FALSE);

	bus = g_bus_own_name(G_BUS_TYPE_SYSTEM,
	                     "org.freedesktop.login1",
	                     G_BUS_NAME_OWNER_FLAGS_NONE,
	                     on_bus_acquired,
	                     on_name_acquired,
	                     on_name_lost,
	                     loop,
	                     NULL);

	/* create /run/systemd/seats, since that's how GDM checks whether logind is
	 * running - see LOGIND_RUNNING() */
	if (-1 == g_mkdir_with_parents("/run/systemd/seats", 0755))
		goto cleanup;

	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_INFO, "entering the main loop");
	g_main_loop_run(loop);
	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_INFO, "exited the main loop");

cleanup:
	signals_unsubscribe();
	bus_close();

	g_main_loop_unref(loop);

	g_bus_unown_name(bus);

	return 0;
}
