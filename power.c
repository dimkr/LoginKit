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
#include "power.h"

void on_handle_can_hybrid_sleep(LoginKitManager *interface,
                                GDBusMethodInvocation *invocation,
                                const gchar *can,
                                gpointer user_data)
{
	/* UPower does not support hybrid sleep */
	g_log(G_LOG_DOMAIN,
	      G_LOG_LEVEL_INFO,
	      "checking whether hybrid sleep is supported");
	login_kit_manager_complete_can_hybrid_sleep(interface, invocation, "na");
}

void on_handle_can_power_off(LoginKitManager *interface,
                             GDBusMethodInvocation *invocation,
                             const gchar *can,
                             gpointer user_data)
{
	/* UPower does not have an equivalent method */
	g_log(G_LOG_DOMAIN,
	      G_LOG_LEVEL_INFO,
	      "checking whether shutdown is supported");
	login_kit_manager_complete_can_power_off(interface, invocation, "yes");
}

void on_handle_can_reboot(LoginKitManager *interface,
                          GDBusMethodInvocation *invocation,
                          const gchar *can,
                          gpointer user_data)
{
	g_log(G_LOG_DOMAIN,
	      G_LOG_LEVEL_INFO,
	      "checking whether reboot is supported");

	login_kit_manager_complete_can_reboot(interface, invocation, "yes");
}

static char *power_can(const char *can_method, const char *allowed_method)
{
	GDBusConnection *bus;
	GVariant *reply;
	GError *error = NULL;
	gboolean result;
	char *ret = "na";

	bus = bus_get();
	if (NULL == bus)
		goto end;

	reply = g_dbus_connection_call_sync(bus,
	                                    "org.freedesktop.UPower",
	                                    "/org/freedesktop/UPower",
	                                    "org.freedesktop.UPower",
	                                    can_method,
	                                    NULL,
	                                    G_VARIANT_TYPE("(b)"),
	                                    G_DBUS_CALL_FLAGS_NONE,
	                                    -1,
	                                    NULL,
	                                    &error);
	if (NULL == reply) {
		if (NULL != error)
			g_error_free(error);
		goto end;
	}

	g_variant_get(reply, "(b)", &result);
	g_variant_unref(reply);

	if (FALSE == result) {
		ret = "no";
		goto end;
	}

	reply = g_dbus_connection_call_sync(bus,
	                                    "org.freedesktop.UPower",
	                                    "/org/freedesktop/UPower",
	                                    "org.freedesktop.UPower",
	                                    allowed_method,
	                                    NULL,
	                                    G_VARIANT_TYPE("(b)"),
	                                    G_DBUS_CALL_FLAGS_NONE,
	                                    -1,
	                                    NULL,
	                                    &error);
	if (NULL == reply) {
		if (NULL != error)
			g_error_free(error);
		goto end;
	}

	g_variant_get(reply, "(b)", &result);
	g_variant_unref(reply);

	if (FALSE == result)
		ret = "challenge";
	else
		ret = "yes";

end:
	return ret;
}

void on_handle_can_suspend(LoginKitManager *interface,
                           GDBusMethodInvocation *invocation,
                           const gchar *can,
                           gpointer user_data)
{
	g_log(G_LOG_DOMAIN,
	      G_LOG_LEVEL_INFO,
	      "checking whether suspending is supported");
	login_kit_manager_complete_can_suspend(interface,
	                                       invocation,
	                                       power_can("CanSuspend",
	                                                 "SuspendAllowed"));
}

void on_handle_can_hibernate(LoginKitManager *interface,
                             GDBusMethodInvocation *invocation,
                             const gchar *can,
                             gpointer user_data)
{
	g_log(G_LOG_DOMAIN,
	      G_LOG_LEVEL_INFO,
	      "checking whether hibernation is supported");
	login_kit_manager_complete_can_hibernate(interface,
	                                         invocation,
	                                         power_can("CanHibernate",
	                                                   "HibernateAllowed"));
}
