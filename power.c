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

static char *handle_can(const char *method)
{
	GDBusConnection *bus;
	GVariant *reply;
	GError *error = NULL;
	char *ret;

	bus = bus_get();
	if (NULL == bus)
		return NULL;

	reply = g_dbus_connection_call_sync(bus,
	                                    "org.freedesktop.ConsoleKit",
	                                    "/org/freedesktop/ConsoleKit/Manager",
	                                    "org.freedesktop.ConsoleKit.Manager",
	                                    method,
	                                    NULL,
	                                    G_VARIANT_TYPE("(s)"),
	                                    G_DBUS_CALL_FLAGS_NONE,
	                                    -1,
	                                    NULL,
	                                    &error);
	if (NULL == reply) {
		if (NULL != error)
			g_error_free(error);
		return NULL;
	}

	g_variant_get(reply, "(s)", &ret);
	g_variant_unref(reply);

	return ret;
}

gboolean on_handle_can_hybrid_sleep(LoginKitManager *interface,
                                    GDBusMethodInvocation *invocation,
                                    const gchar *can,
                                    gpointer user_data)
{
	char *ret;

	g_log(G_LOG_DOMAIN,
	      G_LOG_LEVEL_INFO,
	      "checking whether hybrid sleep is supported");

	ret = handle_can("CanHybridSleep");
	if (NULL == ret)
		return FALSE;

	login_kit_manager_complete_can_hybrid_sleep(interface, invocation, ret);
	return TRUE;
}

gboolean on_handle_can_power_off(LoginKitManager *interface,
                                 GDBusMethodInvocation *invocation,
                                 const gchar *can,
                                 gpointer user_data)
{
	char *ret;

	g_log(G_LOG_DOMAIN,
	      G_LOG_LEVEL_INFO,
	      "checking whether shutdown is supported");

	ret = handle_can("CanPowerOff");
	if (NULL == ret)
		return FALSE;

	login_kit_manager_complete_can_power_off(interface, invocation, ret);
	return TRUE;
}

gboolean on_handle_can_reboot(LoginKitManager *interface,
                              GDBusMethodInvocation *invocation,
                              const gchar *can,
                              gpointer user_data)
{
	char *ret;

	g_log(G_LOG_DOMAIN,
	      G_LOG_LEVEL_INFO,
	      "checking whether reboot is supported");

	ret = handle_can("CanReboot");
	if (NULL == ret)
		return FALSE;

	login_kit_manager_complete_can_reboot(interface, invocation, ret);
	return TRUE;
}

gboolean on_handle_can_suspend(LoginKitManager *interface,
                               GDBusMethodInvocation *invocation,
                               const gchar *can,
                               gpointer user_data)
{
	char *ret;

	g_log(G_LOG_DOMAIN,
	      G_LOG_LEVEL_INFO,
	      "checking whether suspending is supported");

	ret = handle_can("CanSuspend");
	if (NULL == ret)
		return FALSE;

	login_kit_manager_complete_can_suspend(interface, invocation, ret);
	return TRUE;
}

gboolean on_handle_can_hibernate(LoginKitManager *interface,
                                 GDBusMethodInvocation *invocation,
                                 const gchar *can,
                                 gpointer user_data)
{
	char *ret;

	g_log(G_LOG_DOMAIN,
	      G_LOG_LEVEL_INFO,
	      "checking whether hibernation is supported");

	ret = handle_can("CanHibernate");
	if (NULL == ret)
		return FALSE;

	login_kit_manager_complete_can_hibernate(interface, invocation, ret);
	return TRUE;
}

gboolean on_handle_inhibit(LoginKitManager *interface,
                           GDBusMethodInvocation *invocation,
                           const gchar *arg_what,
                           const gchar *arg_who,
                           const gchar *arg_why,
                           const gchar *arg_mode,
                           GVariant **fd,
                           gpointer user_data)
{
	GDBusConnection *bus;
	GVariant *reply;
	GError *error = NULL;
	gint ret;

	g_log(G_LOG_DOMAIN,
	      G_LOG_LEVEL_INFO,
	      "inhibiting %s for %s",
	      arg_what,
	      arg_who);

	bus = bus_get();
	if (NULL == bus)
		return FALSE;

	reply = g_dbus_connection_call_sync(bus,
	                                    "org.freedesktop.ConsoleKit",
	                                    "/org/freedesktop/ConsoleKit/Manager",
	                                    "org.freedesktop.ConsoleKit.Manager",
	                                    "Inhibit",
	                                    g_variant_new("(sss)",
	                                                  arg_what,
	                                                  arg_who,
	                                                  arg_why),
	                                    G_VARIANT_TYPE("(h)"),
	                                    G_DBUS_CALL_FLAGS_NONE,
	                                    -1,
	                                    NULL,
	                                    &error);
	if (NULL == reply) {
		if (NULL != error)
			g_error_free(error);
		return FALSE;
	}

	g_variant_get(reply, "(h)", &ret);
	g_variant_unref(reply);

	login_kit_manager_complete_inhibit(interface,
	                                   invocation,
	                                   g_variant_new("(h)", ret));
	return TRUE;
}

static gboolean handle_action(const char *method, const gboolean interactive)
{
	GDBusConnection *bus;
	GVariant *reply;
	GError *error = NULL;

	bus = bus_get();
	if (NULL == bus)
		return FALSE;

	reply = g_dbus_connection_call_sync(bus,
	                                    "org.freedesktop.ConsoleKit",
	                                    "/org/freedesktop/ConsoleKit/Manager",
	                                    "org.freedesktop.ConsoleKit.Manager",
	                                    method,
	                                    g_variant_new("(b)", interactive),
	                                    NULL,
	                                    G_DBUS_CALL_FLAGS_NONE,
	                                    -1,
	                                    NULL,
	                                    &error);
	if (NULL == reply) {
		if (NULL != error)
			g_error_free(error);
		return FALSE;
	}

	g_variant_unref(reply);

	return TRUE;
}

gboolean on_handle_suspend(LoginKitManager *interface,
                           GDBusMethodInvocation *invocation,
                           const gboolean arg_interactive,
                           gpointer user_data)
{
	gboolean ret;

	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_INFO, "Suspending");

	ret = handle_action("Suspend", arg_interactive);
	if (TRUE == ret)
		login_kit_manager_complete_suspend(interface, invocation);

	return ret;
}

gboolean on_handle_hybrid_sleep(LoginKitManager *interface,
                                GDBusMethodInvocation *invocation,
                                const gboolean arg_interactive,
                                gpointer user_data)
{
	gboolean ret;

	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_INFO, "Entering hybrid sleep");

	ret = handle_action("HybridSleep", arg_interactive);
	if (TRUE == ret)
		login_kit_manager_complete_hybrid_sleep(interface, invocation);

	return ret;
}

gboolean on_handle_power_off(LoginKitManager *interface,
                             GDBusMethodInvocation *invocation,
                             const gboolean arg_interactive,
                             gpointer user_data)
{
	gboolean ret;

	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_INFO, "Shutting down");

	ret = handle_action("PowerOff", arg_interactive);
	if (TRUE == ret)
		login_kit_manager_complete_suspend(interface, invocation);

	return ret;
}

gboolean on_handle_reboot(LoginKitManager *interface,
                          GDBusMethodInvocation *invocation,
                          const gboolean arg_interactive,
                          gpointer user_data)
{
	gboolean ret;

	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_INFO, "Rebooting");

	ret = handle_action("Reboot", arg_interactive);
	if (TRUE == ret)
		login_kit_manager_complete_suspend(interface, invocation);

	return ret;
}

gboolean on_handle_hibernate(LoginKitManager *interface,
                             GDBusMethodInvocation *invocation,
                             const gboolean arg_interactive,
                             gpointer user_data)
{
	gboolean ret;

	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_INFO, "Hibernating");

	ret = handle_action("Hibernate", arg_interactive);
	if (TRUE == ret)
		login_kit_manager_complete_suspend(interface, invocation);

	return ret;
}
