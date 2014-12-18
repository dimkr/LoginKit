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

#include <stdio.h>
#include <syslog.h>
#include <errno.h>
#include <string.h>

#include "journal.h"

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
