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

#ifndef _LOGINKIT_JOURNAL_H_INCLUDED
#	define _LOGINKIT_JOURNAL_H_INCLUDED

#	include <stdarg.h>
#	include <signal.h>

#	define MAX_FMT_LEN (1024)
#	define MAX_MSG_LEN (2048)

struct stream_params {
	sigset_t mask;
	const char *identifier;
	int priority;
	int fd;
};

int sd_journal_printv(int priority, const char *format, va_list ap);
int sd_journal_print(int priority, const char *format, ...);
int sd_journal_print_with_location(int priority,
                                   const char *file,
                                   const char *line,
                                   const char *func,
                                   const char *format,
                                   ...);

/* this function is problematic, because /dev/log is a datagram Unix socket;
 * therefore, it creates a pair of connected stream sockets and spawns a thread
 * that reads messages sent through the socket, then relays them to syslogd
 * using syslog() */
int sd_journal_stream_fd(const char *identifier,
                         int priority,
                         int level_prefix);

int sd_journal_perror(const char *message);


#endif
