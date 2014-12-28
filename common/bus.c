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

#include <pthread.h>

#include <glib.h>

#include <common/bus.h>

static pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;
static GDBusConnection *g_bus = NULL;

GDBusConnection *bus_get(void)
{
	GError *error = NULL;

	if (0 != pthread_mutex_lock(&g_lock))
		return NULL;

	if (NULL == g_bus) {
		g_log(G_LOG_DOMAIN, G_LOG_LEVEL_INFO, "connecting to the system bus");
		g_bus = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &error);
		if (NULL == g_bus) {
			g_log(G_LOG_DOMAIN,
			      G_LOG_LEVEL_CRITICAL,
			      "failed to connect to the system bus: %s",
			      error->message);
			g_error_free(error);
		}
	}

	(void) pthread_mutex_unlock(&g_lock);

	return g_bus;
}

void bus_close(void)
{
	if (0 != pthread_mutex_lock(&g_lock))
		return;

	if (NULL != g_bus) {
		g_log(G_LOG_DOMAIN,
		      G_LOG_LEVEL_INFO,
		      "disconnecting from the system bus");
		(void) g_dbus_connection_close_sync(g_bus, NULL, NULL);
		g_object_unref(g_bus);
		g_bus = NULL;
	}

	(void) pthread_mutex_unlock(&g_lock);

	(void) pthread_mutex_destroy(&g_lock);
}
