#include <string.h>
#include <errno.h>
#include <pthread.h>

#include <gio/gio.h>

#include "loginkit.h"

static pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;
static GDBusConnection *g_bus = NULL;

static GDBusConnection *get_bus(void)
{
	GError *error;

	if (0 != pthread_mutex_lock(&g_lock))
		return NULL;

	if (NULL == g_bus) {
		g_bus = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &error);
		if ((NULL == g_bus) && (NULL != error))
			g_error_free(error);
	}

	(void) pthread_mutex_unlock(&g_lock);

	return g_bus;
}

static void close_bus(void)
{
	if (0 != pthread_mutex_lock(&g_lock))
		return;

	(void) g_dbus_connection_close_sync(g_bus, NULL, NULL);
	g_object_unref(g_bus);
	g_bus = NULL;

	(void) pthread_mutex_unlock(&g_lock);
}

__attribute__((destructor))
static void on_dlclose(void)
{
	close_bus();
	(void) pthread_mutex_destroy(&g_lock);
}

static char *get_session_by_pid(GDBusConnection *bus, pid_t pid)
{
	GVariant *reply;
	GError *error = NULL;
	char *ret = NULL;

	if (0 == pid)
		pid = getpid();

	reply = g_dbus_connection_call_sync(bus,
	                                    "org.freedesktop.ConsoleKit",
	                                    "/org/freedesktop/ConsoleKit/Manager",
	                                    "org.freedesktop.ConsoleKit.Manager",
	                                    "GetSessionForUnixProcess",
	                                    g_variant_new("(u)", pid),
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
int sd_pid_get_session_by_pid(pid_t pid, char **session)
{
	GDBusConnection *bus;

	bus = get_bus();
	if (NULL == bus) {
		*session = NULL;
		return -EINVAL;
	}

	*session = get_session_by_pid(bus, pid);
	if (NULL == *session)
		return -EINVAL;

	return 0;
}

__attribute__((visibility("default")))
int sd_pid_get_owner_uid(pid_t pid, uid_t *uid)
{
	GDBusConnection *bus;
	GVariant *reply;
	GError *error = NULL;
	char *session;

	bus = get_bus();
	if (NULL == bus)
		return -EINVAL;

	session = get_session_by_pid(bus, pid);
	if (NULL == session)
		return -EINVAL;

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
		return -EINVAL;
	}

	g_variant_get(reply, "(u)", uid);
	g_variant_unref(reply);

	return 0;
}

__attribute__((visibility("default")))
int sd_pid_get_machine_name(pid_t pid, char **name)
{
	GDBusConnection *bus;
	GVariant *reply;
	GError *error = NULL;
	char *session;

	*name = NULL;

	bus = get_bus();
	if (NULL == bus)
		return -EINVAL;

	session = get_session_by_pid(bus, pid);
	if (NULL == session)
		return -EINVAL;

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
		return -EINVAL;
	}

	g_variant_get(reply, "(s)", name);
	g_variant_unref(reply);

	return 0;
}

__attribute__((visibility("default")))
int sd_pid_get_unit(pid_t pid, char **unit)
{
	/* quote from the man page: "For processes not being part of a systemd
	 * system unit this function will fail." */
	return -EINVAL;
}

__attribute__((visibility("default")))
int sd_pid_get_user_unit(pid_t pid, char **unit)
{
	return -EINVAL;
}

__attribute__((visibility("default")))
int sd_pid_get_slice(pid_t pid, char **slice)
{
	return -EINVAL;
}
