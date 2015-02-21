/*
 * this file is part of LoginKit.
 *
 * Copyright (c) 2014, 2015 Dima Krasner
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

#include "loginkitd-generated.h"

gboolean on_handle_inhibit(LoginKitManager *interface,
                           GDBusMethodInvocation *invocation,
                           const gchar *arg_what,
                           const gchar *arg_who,
                           const gchar *arg_why,
                           const gchar *arg_mode,
                           GVariant **fd,
                           gpointer user_data)
{
	g_log(G_LOG_DOMAIN,
	      G_LOG_LEVEL_INFO,
	      "inhibiting %s for %s",
	      arg_what,
	      arg_who);

	login_kit_manager_complete_inhibit(interface,
	                                   invocation,
	                                   g_variant_new("h", -1));
	return TRUE;
}

static void on_bus_acquired(GDBusConnection *bus,
                            const gchar *name,
                            gpointer user_data)
{
	LoginKitManager *interface;
	GError *error = NULL;

	interface = login_kit_manager_skeleton_new();

	g_signal_connect(interface,
	                 "handle-inhibit",
	                 G_CALLBACK(on_handle_inhibit),
	                 NULL);

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

	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_INFO, "entering the main loop");
	g_main_loop_run(loop);
	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_INFO, "exited the main loop");

	g_main_loop_unref(loop);

	g_bus_unown_name(bus);

	return 0;
}
