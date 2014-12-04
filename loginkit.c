#include <string.h>
#include <errno.h>

#include <gio/gio.h>

#include "loginkit.h"

__attribute__((visibility("default")))
int sd_pid_get_session(pid_t pid, char **session)
{
	GDBusConnection *bus;
	GVariant *reply;
	GError *error;
	int ret = -EINVAL;

	*session = NULL;

	bus = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &error);
	if (NULL == bus) {
		g_error_free(error);
		goto end;
	}

	reply = g_dbus_connection_call_sync(bus,
	                                    "org.freedesktop.ConsoleKit",
	                                    "/org/freedesktop/ConsoleKit/Manager",
	                                    "org.freedesktop.ConsoleKit.Manager",
	                                    "GetCurrentSession",
	                                    NULL,
	                                    G_VARIANT_TYPE("(o)"),
	                                    G_DBUS_CALL_FLAGS_NONE,
	                                    -1,
	                                    NULL,
	                                    &error);
	if (NULL == reply) {
		g_error_free(error);
		goto disconnect;
	}

	g_variant_get(reply, "(o)", session);
	g_variant_unref(reply);

	ret = 0;

disconnect:
	(void) g_dbus_connection_close_sync(bus, NULL, NULL);
	g_object_unref(bus);

end:
	return ret;
}

static char *get_active_session(GDBusConnection *bus)
{
	GVariant *reply;
	GError *error = NULL;
	char *ret = NULL;

	reply = g_dbus_connection_call_sync(bus,
	                                    "org.freedesktop.ConsoleKit",
	                                    "/org/freedesktop/ConsoleKit/Manager",
	                                    "org.freedesktop.ConsoleKit.Manager",
	                                    "GetCurrentSession",
	                                    NULL,
	                                    G_VARIANT_TYPE("(o)"),
	                                    G_DBUS_CALL_FLAGS_NONE,
	                                    -1,
	                                    NULL,
	                                    &error);

	if (NULL == reply)
		g_error_free(error);
	else {
		g_variant_get(reply, "(o)", &ret);
		g_variant_unref(reply);
	}

	return ret;
}

__attribute__((visibility("default")))
int sd_pid_get_owner_uid(pid_t pid, uid_t *uid)
{
	GDBusConnection *bus;
	GVariant *reply;
	GError *error = NULL;
	char *session;
	int ret = -EINVAL;

	bus = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &error);
	if (NULL == bus) {
		if (NULL != error)
			g_error_free(error);
		goto end;
	}

	session = get_active_session(bus);
	if (NULL == session)
		goto disconnect;

	reply = g_dbus_connection_call_sync(bus,
	                                    "org.freedesktop.ConsoleKit",
	                                    session,
	                                    "org.freedesktop.ConsoleKit.Session",
	                                    "GetUnixUser",
	                                    NULL,
	                                    G_VARIANT_TYPE("(u)"),
	                                    G_DBUS_CALL_FLAGS_NONE,
	                                    -1,
	                                    NULL,
	                                    &error);
	if (NULL == reply) {
		if (NULL != error)
			g_error_free(error);
		goto disconnect;
	}

	g_variant_get(reply, "(u)", uid);
	g_variant_unref(reply);

	ret = 0;

disconnect:
	(void) g_dbus_connection_close_sync(bus, NULL, NULL);
	g_object_unref(bus);

end:
	return ret;
}

__attribute__((visibility("default")))
int sd_pid_get_machine_name(pid_t pid, char **name)
{
	GDBusConnection *bus;
	GVariant *reply;
	GError *error = NULL;
	char *session;
	int ret = -EINVAL;

	*name = NULL;

	bus = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &error);
	if (NULL == bus) {
		if (NULL != error)
			g_error_free(error);
		goto end;
	}

	session = get_active_session(bus);
	if (NULL == session)
		goto disconnect;

	reply = g_dbus_connection_call_sync(bus,
	                                    "org.freedesktop.ConsoleKit",
	                                    session,
	                                    "org.freedesktop.ConsoleKit.Session",
	                                    "GetRemoteHostName",
	                                    NULL,
	                                    G_VARIANT_TYPE("(s)"),
	                                    G_DBUS_CALL_FLAGS_NONE,
	                                    -1,
	                                    NULL,
	                                    &error);
	if (NULL == reply) {
		if (NULL != error)
			g_error_free(error);
		goto disconnect;
	}

	g_variant_get(reply, "(s)", name);
	g_variant_unref(reply);

	ret = 0;

disconnect:
	(void) g_dbus_connection_close_sync(bus, NULL, NULL);
	g_object_unref(bus);

end:
	return ret;
}

__attribute__((visibility("default")))
int sd_pid_get_unit(pid_t pid, char **unit)
{
	/* quote from the man page: "For processes not being part of a systemd
	 * system unit this function will fail." */
	return -EINVAL;
}
