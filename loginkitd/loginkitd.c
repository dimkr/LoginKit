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
#include "session.h"
#include "power.h"
#include "loginkitd-generated.h"

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
	g_signal_connect(interface,
	                 "handle-get-session",
	                 G_CALLBACK(on_handle_get_session),
	                 NULL);

	g_signal_connect(interface,
	                 "handle-can-hybrid-sleep",
	                 G_CALLBACK(on_handle_can_hybrid_sleep),
	                 NULL);
	g_signal_connect(interface,
	                 "handle-can-power-off",
	                 G_CALLBACK(on_handle_can_power_off),
	                 NULL);
	g_signal_connect(interface,
	                 "handle-can-reboot",
	                 G_CALLBACK(on_handle_can_reboot),
	                 NULL);
	g_signal_connect(interface,
	                 "handle-can-suspend",
	                 G_CALLBACK(on_handle_can_suspend),
	                 NULL);
	g_signal_connect(interface,
	                 "handle-can-hibernate",
	                 G_CALLBACK(on_handle_can_hibernate),
	                 NULL);
	g_signal_connect(interface,
	                 "handle-inhibit",
	                 G_CALLBACK(on_handle_inhibit),
	                 NULL);
	g_signal_connect(interface,
	                 "handle-suspend",
	                 G_CALLBACK(on_handle_suspend),
	                 NULL);
	g_signal_connect(interface,
	                 "handle-suspend",
	                 G_CALLBACK(on_handle_suspend),
	                 NULL);
	g_signal_connect(interface,
	                 "handle-hybrid-sleep",
	                 G_CALLBACK(on_handle_hybrid_sleep),
	                 NULL);
	g_signal_connect(interface,
	                 "handle-power-off",
	                 G_CALLBACK(on_handle_power_off),
	                 NULL);
	g_signal_connect(interface,
	                 "handle-reboot",
	                 G_CALLBACK(on_handle_reboot),
	                 NULL);
	g_signal_connect(interface,
	                 "handle-hibernate",
	                 G_CALLBACK(on_handle_hibernate),
	                 NULL);

	signals_subscribe(interface);

	if (!g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(interface),
	                                      bus,
	                                      "/org/freedesktop/login1",
	                                      &error))
		g_error_free(error);
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
	if (-1 == g_mkdir_with_parents("/run/systemd/sessions", 0755))
		goto cleanup;

	/* normally, /run/systemd/multi-session-x contains a file for each session,
	 * but since ConsoleKit's session IDs are D-Bus object paths, they contain /
	 * characeters, so extra sub-directories are required to prevent failure of
	 * mkdir() in LoginKit clients */
	if (-1 == g_mkdir_with_parents(
	                  "/run/systemd/multi-session-x/org/freedesktop/ConsoleKit",
	                  0755))
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
