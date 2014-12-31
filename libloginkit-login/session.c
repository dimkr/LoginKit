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

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <common/bus.h>
#include "pid.h"
#include "session.h"

typedef int (*get_t)(const char *session, void *out);

static int is_active(const char *session)
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
		g_log(G_LOG_DOMAIN,
		      G_LOG_LEVEL_ERROR,
		      "IsActive() failed: %s",
		      error->message);
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
int sd_session_is_active(const char *session)
{
	char *real_session;
	int ret;

	if (NULL != session)
		return is_active(session);

	ret = sd_pid_get_session(0, &real_session);
	if (0 != ret)
		return ret;

	ret = is_active(real_session);
	free(real_session);
	return ret;
}

static int wrapper(const char *session, void *out, const get_t get)
{
	char *real_session;
	int ret;

	if (NULL != session)
		return get(session, out);

	ret = sd_pid_get_session(0, &real_session);
	if (0 != ret)
		return ret;

	ret = get(real_session, out);
	free(real_session);
	return ret;
}

static int get_type(const char *session, char **type)
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
		g_log(G_LOG_DOMAIN,
		      G_LOG_LEVEL_ERROR,
		      "GetSessionType() failed: %s",
		      error->message);
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
int sd_session_get_type(const char *session, char **type)
{
	return wrapper(session, type, (const get_t) get_type);
}

static int get_seat(const char *session, char **seat)
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
		g_log(G_LOG_DOMAIN,
		      G_LOG_LEVEL_ERROR,
		      "GetSeatId() failed: %s",
		      error->message);
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
int sd_session_get_seat(const char *session, char **seat)
{
	return wrapper(session, seat, (const get_t) get_seat);
}

static int get_uid(const char *session, uid_t *uid)
{
	GDBusConnection *bus;
	GVariant *reply;
	GError *error = NULL;

	g_assert(NULL != uid);

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
		g_log(G_LOG_DOMAIN,
		      G_LOG_LEVEL_ERROR,
		      "GetUnixUser() failed: %s",
		      error->message);
		g_error_free(error);
		return -EINVAL;
	}

	g_variant_get(reply, "(u)", uid);
	g_variant_unref(reply);

	return 0;
}

__attribute__((visibility("default")))
int sd_session_get_uid(const char *session, uid_t *uid)
{
	return wrapper(session, uid, (const get_t) get_uid);
}

__attribute__((visibility("default")))
int sd_session_get_state(const char *session, char **state)
{
	int ret;

	g_assert(NULL != state);

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
int sd_session_get_class(const char *session, char **class)
{
	g_assert(NULL != class);

	/* ConsoleKit does not have session classes, so we assume all sessions are
	 * "user" ones */
	*class = strndup("user", 4);
	if (NULL == *class)
		return -ENOMEM;

	return 0;
}
