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
#include "seat.h"

char *seat_get_id(GDBusConnection *bus, const char *seat)
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
gboolean on_handle_list_seats(LoginKitManager *interface,
                              GDBusMethodInvocation *invocation,
                              gpointer user_data)
{
	GVariantIter iter;
	GVariant *reply;
	GError *error = NULL;
	GDBusConnection *bus;
	GVariantBuilder builder;
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
	g_variant_builder_init(&builder, G_VARIANT_TYPE("a(so)"));

	/* do a best-effort attempt to enumerate all seats */
	while (TRUE == g_variant_iter_loop(&iter, "o", &seat, NULL)) {
		id = seat_get_id(bus, seat);
		if (NULL == id)
			continue;
		g_variant_builder_add(&builder, "(so)", id, seat);
	}

	g_variant_unref(reply);
	g_variant_unref(seats);

	ret = g_variant_builder_end(&builder);

	login_kit_manager_complete_list_seats(interface, invocation, ret);

	return TRUE;
}

void on_seat_added(GDBusConnection *connection,
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

	id = seat_get_id(bus, seat);
	if (NULL == id)
		return;

	login_kit_manager_emit_seat_new((LoginKitManager *) user_data, id, seat);
}

void on_seat_removed(GDBusConnection *connection,
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

	id = seat_get_id(bus, seat);
	if (NULL == id)
		return;

	login_kit_manager_emit_seat_removed((LoginKitManager *) user_data,
	                                    id,
	                                    seat);
}
