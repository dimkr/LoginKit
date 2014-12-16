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

#include <string.h>
#include <errno.h>

#include "bus.h"
#include "loginkit.h"

__attribute__((destructor))
static void on_dlclose(void)
{
	bus_close();
}

static char *get_session_by_pid(GDBusConnection *bus, pid_t pid)
{
	GVariant *reply;
	GError *error = NULL;
	char *ret = NULL;

	if (0 == pid)
		pid = getpid();

	g_log(G_LOG_DOMAIN,
	      G_LOG_LEVEL_INFO,
	      "getting the session of process %ld",
	      (long) pid);

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
int sd_pid_get_session(pid_t pid, char **session)
{
	GDBusConnection *bus;

	bus = bus_get();
	if (NULL == bus) {
		*session = NULL;
		return -EINVAL;
	}

	*session = get_session_by_pid(bus, pid);
	if (NULL == *session)
		return -EINVAL;

	*session = strdup(*session);
	if (NULL == *session)
		return -ENOMEM;

	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_INFO, "the session is %s", *session);

	return 0;
}

__attribute__((visibility("default")))
int sd_pid_get_owner_uid(pid_t pid, uid_t *uid)
{
	GDBusConnection *bus;
	GVariant *reply;
	GError *error = NULL;
	char *session;

	bus = bus_get();
	if (NULL == bus)
		return -EINVAL;

	session = get_session_by_pid(bus, pid);
	if (NULL == session)
		return -EINVAL;

	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_INFO, "getting the owner of %s", session);

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

	bus = bus_get();
	if (NULL == bus)
		return -EINVAL;

	session = get_session_by_pid(bus, pid);
	if (NULL == session)
		return -EINVAL;

	g_log(G_LOG_DOMAIN,
	      G_LOG_LEVEL_INFO,
	      "getting the hostname of %s",
	      session);

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
int sd_seat_can_multi_session(const char *seat)
{
	/* let's assume very seat is multi-session, since there's no ConsoleKit
	 * interface for this */
	return 1;
}

__attribute__((visibility("default")))
int sd_session_is_active(const char *session)
{
	GDBusConnection *bus;
	GVariant *reply;
	GError *error = NULL;
	gboolean active;

	g_log(G_LOG_DOMAIN,
	      G_LOG_LEVEL_INFO,
	      "checking whether %s is active",
	      session);

	bus = bus_get();
	if (NULL == bus)
		return -EINVAL;

	reply = g_dbus_connection_call_sync(bus,
	                                    "org.freedesktop.ConsoleKit",
	                                    session,
	                                    "org.freedesktop.ConsoleKit.Session",
	                                    "IsActive",
	                                    NULL,
	                                    G_VARIANT_TYPE("(b)"),
	                                    G_DBUS_CALL_FLAGS_NONE,
	                                    -1,
	                                    NULL,
	                                    &error);
	if (NULL == reply) {
		if (NULL != error)
			g_error_free(error);
		return -EINVAL;
	}

	g_variant_get(reply, "(b)", &active);
	g_variant_unref(reply);

	if (FALSE == active)
		return 0;

	return 1;
}

__attribute__((visibility("default")))
int sd_session_get_state(const char *session, char **state)
{
	int ret;

	ret = sd_session_is_active(session);
	switch (ret) {
		case 0:
			*state = strdup("online");
			if (NULL == *state)
				return -ENOMEM;
			break;

		case 1:
			*state = strdup("active");
			if (NULL == *state)
				return -ENOMEM;
			break;

		default:
			*state = NULL;
			return ret;
	}

	return 0;
}

__attribute__((visibility("default")))
int sd_session_get_type(const char *session, char **type)
{
	GDBusConnection *bus;
	GVariant *reply;
	GError *error = NULL;

	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_INFO, "getting the type of %s", session);

	bus = bus_get();
	if (NULL == bus)
		return -EINVAL;

	reply = g_dbus_connection_call_sync(bus,
	                                    "org.freedesktop.ConsoleKit",
	                                    session,
	                                    "org.freedesktop.ConsoleKit.Session",
	                                    "GetSessionType",
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

	g_variant_get(reply, "(s)", type);
	g_variant_unref(reply);

	*type = strdup(*type);
	if (NULL == *type)
		return -ENOMEM;

	return 0;
}

__attribute__((visibility("default")))
int sd_session_get_seat(const char *session, char **seat)
{
	GDBusConnection *bus;
	GVariant *reply;
	GError *error = NULL;

	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_INFO, "getting the seat of %s", session);

	bus = bus_get();
	if (NULL == bus)
		return -EINVAL;

	reply = g_dbus_connection_call_sync(bus,
	                                    "org.freedesktop.ConsoleKit",
	                                    session,
	                                    "org.freedesktop.ConsoleKit.Session",
	                                    "GetSeatId",
	                                    NULL,
	                                    G_VARIANT_TYPE("(o)"),
	                                    G_DBUS_CALL_FLAGS_NONE,
	                                    -1,
	                                    NULL,
	                                    &error);
	if (NULL == reply) {
		if (NULL != error)
			g_error_free(error);
		return -EINVAL;
	}

	g_variant_get(reply, "(o)", seat);
	g_variant_unref(reply);

	*seat = strdup(*seat);
	if (NULL == *seat)
		return -ENOMEM;

	return 0;
}

__attribute__((visibility("default")))
int sd_session_get_uid(const char *session, uid_t *uid)
{
	GDBusConnection *bus;
	GVariant *reply;
	GError *error = NULL;

	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_INFO, "getting the owner of %s", session);

	bus = bus_get();
	if (NULL == bus)
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
int sd_seat_get_sessions(const char *seat,
                         char ***sessions,
                         uid_t **uid,
                         unsigned *n_uids)
{
	GVariantIter iter;
	GVariant *array;
	char *session;
	GDBusConnection *bus;
	GVariant *reply;
	GError *error = NULL;
	uid_t owner;
	int ret = -EINVAL;
	unsigned i;

	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_INFO, "listing the sessions on %s", seat);

	bus = bus_get();
	if (NULL == bus)
		goto end;

	reply = g_dbus_connection_call_sync(bus,
	                                    "org.freedesktop.ConsoleKit",
	                                    seat,
	                                    "org.freedesktop.ConsoleKit.Seat",
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
		goto end;
	}

	array = g_variant_get_child_value(reply, 0);
	g_variant_iter_init(&iter, array);

	*n_uids = (unsigned) g_variant_iter_n_children(&iter);
	*sessions = g_new(char *, 1 + (*n_uids));
	*uid = g_new(uid_t, *n_uids);

	for (i = 0; (*n_uids) > i; ++i) {
		(void) g_variant_iter_loop(&iter, "o", &session, NULL);
		ret = sd_session_get_uid(session, &owner);
		if (0 != ret) {
			g_free(*sessions);
			g_free(*uid);
			*sessions = NULL;
			*uid = NULL;
			break;
		}

		*sessions[i] = session;
		*uid[i] = owner;
	}

	ret = 0;

	/* the session IDs array must have a terminating NULL sentinel */
	sessions[i] = NULL;

	g_variant_unref(reply);
	g_variant_unref(array);

end:
	return ret;
}

__attribute__((visibility("default")))
int sd_booted(void)
{
	/* no, systemd is not PID 1 */
	return 0;
}
