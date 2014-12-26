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
#include <stdio.h>
#include <syslog.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <poll.h>
#include <pthread.h>
#include <unistd.h>

#include "sd-journal.h"

/*
 * the journal_print documentation says: "syslog(3) and sd_journal_print() may
 * largely be used interchangeably functionality-wise", so LoginKit's
 * sd_journal_print() and sd_journal_perror() call sd_journal_printv()
 * internally, while the latter is nothing but a vsyslog() proxy.
 */

__attribute__((visibility("default")))
int sd_journal_printv(int priority, const char *format, va_list ap)
{
	vsyslog(priority, format, ap);
	return 0;
}

__attribute__((visibility("default")))
int sd_journal_print(int priority, const char *format, ...)
{
	va_list ap;
	int ret;

	va_start(ap, format);
	ret = sd_journal_printv(priority, format, ap);
	va_end(ap);

	return ret;
}

__attribute__((visibility("default")))
int sd_journal_print_with_location(int priority,
                                   const char *file,
                                   const char *line,
                                   const char *func,
                                   const char *format,
                                   ...)
{
	char fmt[MAX_FMT_LEN];
	va_list ap;
	int len;
	int ret;

	len = snprintf(fmt,
	               sizeof(fmt),
	               "file=%s line=%s func=%s %s: %%s",
	               file,
	               line,
	               func,
	               format);
	if ((0 >= len) || (sizeof(fmt) <= len))
		return -EINVAL;

	va_start(ap, format);
	ret = sd_journal_printv(priority, fmt, ap);
	va_end(ap);

	return ret;
}

__attribute__((visibility("default")))
int sd_journal_perror(const char *message)
{
	char fmt[MAX_FMT_LEN];
	int len;
	char *err;

	err = strerror(errno);
	if (NULL == err)
		return -EINVAL;

	if (NULL == message)
		return sd_journal_print(LOG_ERR, err);

	len = snprintf(fmt, sizeof(fmt), "%s: %%s", message);
	if ((0 >= len) || (sizeof(fmt) <= len))
		return -EINVAL;

	return sd_journal_print(LOG_ERR, fmt, err);
}

static void cleanup(void *arg)
{
	struct stream_params *params = (struct stream_params *) arg;

	(void) close(params->fd);
	free(arg);
}

static void *relay(void *arg)
{
	char message[MAX_MSG_LEN];
	char *off;
	struct pollfd pfd;
	struct stream_params *params = (struct stream_params *) arg;
	ssize_t len;

	pthread_cleanup_push(cleanup, arg);

	if (0 != pthread_sigmask(SIG_BLOCK, &params->mask, NULL))
		goto end;

	pfd.fd = params->fd;
	pfd.events = POLLIN | POLLERR;
	do {
		pfd.revents = 0;
		if (1 != poll(&pfd, 1, -1))
			break;

		if (0 != (POLLERR & pfd.revents))
			break;

		len = recv(pfd.fd, (void *) message, sizeof(message) - 1, 0);
		if (0 >= len)
			break;
		message[len] = '\0';

		/* if the message begins <%d> (one of the SD_* constants), strip it */
		off = strchr(message, '>');
		if (NULL == off)
			off = message;
		else
			++off;

		if (0 != sd_journal_print(params->priority,
		                          "%s: %s",
		                          params->identifier,
		                          off))
			break;
	} while (1);

end:
	pthread_cleanup_pop(1);

	pthread_exit(NULL);
}

__attribute__((visibility("default")))
int sd_journal_stream_fd(const char *identifier,
                         int priority,
                         int level_prefix)
{
	pthread_attr_t attr;
	struct stream_params *params;
	int fds[2];
	pthread_t id;
	int ret = -ENOMEM;

	if (0 != pthread_attr_init(&attr))
		goto end;

	if (0 != pthread_attr_setdetachstate(&attr, 1))
		goto destroy_attr;

	params = (struct stream_params *) malloc(sizeof(struct stream_params));
	if (NULL == params)
		goto destroy_attr;

	if (-1 == sigfillset(&params->mask))
		goto free_params;

	/* create a pair of connected sockets */
	if (-1 == socketpair(AF_UNIX, SOCK_DGRAM, 0, fds))
		goto free_params;

	/* create a detached thread that reads the messages sent through the socket
	 * and passes them to syslog() */
	params->identifier = identifier;
	params->priority = priority;
	params->fd = fds[1];
	if (0 != pthread_create(&id, &attr, relay, (void *) params))
		goto free_params;

	ret = fds[0];
	goto destroy_attr;

free_params:
	free((void *) params);

destroy_attr:
	(void) pthread_attr_destroy(&attr);

end:
	return ret;
}
