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

#include "bus.h"
#include "session.h"
#include "seat.h"

__attribute__((visibility("default")))
int sd_seat_can_multi_session(const char *seat)
{
	/* let's assume very seat is multi-session, since there's no ConsoleKit
	 * interface for this */
	return 1;
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
