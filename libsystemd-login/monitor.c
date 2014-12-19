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

#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/inotify.h>
#include <poll.h>
#include <errno.h>

#include <glib.h>

#include "monitor.h"

__attribute__((visibility("default")))
int sd_login_monitor_new(const char *category,
                         sd_login_monitor **ret)
{
	char path[PATH_MAX];
	int len;
	const char *real_category;

	*ret = malloc(sizeof(sd_login_monitor));
	if (NULL == *ret)
		goto error;

	(*ret)->fd = inotify_init1(IN_CLOEXEC);
	if (-1 == (*ret)->fd)
		goto free_monitor;

	if (NULL == category)
		real_category = "seat";
	else
		real_category = category;

	g_log(G_LOG_DOMAIN,
	      G_LOG_LEVEL_INFO,
	      "creating a %s monitor",
	      real_category);

	len = snprintf(path, sizeof(path), "/run/systemd/%ss", real_category);
	if ((0 >= len) || (sizeof(path) <= len))
		goto free_monitor;

	(*ret)->wd = inotify_add_watch((*ret)->fd, path, IN_CREATE | IN_DELETE);
	if (-1 == (*ret)->wd)
		goto free_monitor;

	g_log(G_LOG_DOMAIN,
	      G_LOG_LEVEL_INFO,
	      "the %s monitor ID is %d",
	      category,
	      (*ret)->fd);

	return 0;

free_monitor:
	free((void *) *ret);

error:
	return -ENOMEM;
}

__attribute__((visibility("default")))
sd_login_monitor *sd_login_monitor_unref(sd_login_monitor *m)
{
	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_INFO, "closing monitor %d", m->fd);
	(void) inotify_rm_watch(m->fd, m->wd);
	free((void *) m);

	return (sd_login_monitor *) NULL;
}

__attribute__((visibility("default")))
int sd_login_monitor_flush(sd_login_monitor *m)
{
	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_INFO, "flushing monitor %d", m->fd);
	return 0;
}

__attribute__((visibility("default")))
int sd_login_monitor_get_fd(sd_login_monitor *m)
{
	g_log(G_LOG_DOMAIN,
	      G_LOG_LEVEL_INFO,
	      "getting the file descriptor of monitor %d",
	      m->wd);
	return m->wd;
}

__attribute__((visibility("default")))
int sd_login_monitor_get_events(sd_login_monitor *m)
{
	g_log(G_LOG_DOMAIN,
	      G_LOG_LEVEL_INFO,
	      "getting the event mask of monitor %d",
	      m->fd);
	return (POLLIN | POLLERR);
}

__attribute__((visibility("default")))
int sd_login_monitor_get_timeout(sd_login_monitor *m, uint64_t *timeout_usec)
{
	g_log(G_LOG_DOMAIN,
	      G_LOG_LEVEL_INFO,
	      "getting the event timeout of monitor %d",
	      m->fd);
	*timeout_usec = (uint64_t) -1;
	return 0;
}
