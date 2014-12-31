#include <errno.h>

#include <common/bus.h>
#include "session.h"
#include "uid.h"

__attribute__((visibility("default")))
int sd_uid_get_seats(uid_t uid,
                     int require_active,
                     char ***seats)
{
	GVariantIter iter;
	GVariant *array;
	char *session;
	char *seat;
	GDBusConnection *bus;
	GVariant *reply;
	GVariant *seat_reply;
	GError *error = NULL;
	gsize count;
	gsize i;
	int ret = -EINVAL;

	g_assert(NULL != seats);

	g_log(G_LOG_DOMAIN,
	      G_LOG_LEVEL_INFO,
	      "listing the sessions of %lu",
	      (unsigned long) uid);

	bus = bus_get();
	if (NULL == bus)
		goto end;

	reply = g_dbus_connection_call_sync(bus,
	                                    "org.freedesktop.ConsoleKit",
	                                    "/org/freedesktop/ConsoleKit/Manager",
	                                    "org.freedesktop.ConsoleKit.Manager",
	                                    "GetSessionsForUnixUser",
	                                    g_variant_new("(u)", uid),
	                                    G_VARIANT_TYPE("(ao)"),
	                                    G_DBUS_CALL_FLAGS_NONE,
	                                    -1,
	                                    NULL,
	                                    &error);
	if (NULL == reply) {
		g_log(G_LOG_DOMAIN,
		      G_LOG_LEVEL_ERROR,
		      "GetSessionsForUnixUser() failed: %s",
		      error->message);
		g_error_free(error);
		goto end;
	}

	array = g_variant_get_child_value(reply, 0);
	g_variant_iter_init(&iter, array);

	count = (unsigned) g_variant_iter_n_children(&iter);
	*seats = g_new(char *, 1 + count);

	for (i = 0; count > i; ++i) {
		(void) g_variant_iter_loop(&iter, "o", &session, NULL);

		if (0 != require_active) {
			ret = sd_session_is_active(session);
			switch (ret) {
				case 0:
					continue;

				case 1:
					break;

				default:
					g_free(*seats);
					*seats = NULL;
					goto end;
			}
		}

		seat_reply = g_dbus_connection_call_sync(
		                                   bus,
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
		if (NULL == seat_reply) {
			g_log(G_LOG_DOMAIN,
			      G_LOG_LEVEL_ERROR,
			      "GetSeatId() failed: %s",
			      error->message);
			g_error_free(error);
			g_free(*seats);
			*seats = NULL;
			goto end;
		}

		g_variant_get(seat_reply, "(o)", &seat);
		g_variant_unref(seat_reply);

		(*seats)[i] = seat;
	}

	ret = 0;

	/* the seats IDs array must have a terminating NULL sentinel */
	(*seats)[i] = NULL;

	g_variant_unref(reply);
	g_variant_unref(array);

end:
	return ret;
}
