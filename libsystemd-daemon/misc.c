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

#include <stddef.h>

#include <glib.h>

#include "misc.h"

__attribute__((visibility("default")))
int sd_booted(void)
{
	/* no, systemd is not PID 1 */
	return 0;
}

/* quote from the sd_notify() man page: "In order to support both, init systems
 * that implement this scheme and those which do not, it is generally
 * recommended to ignore the return value of this call." */
__attribute__((visibility("default")))
int sd_notify(int unset_environment, const char *state)
{
	g_assert(NULL != state);

	return 0;
}

__attribute__((visibility("default")))
int sd_notifyf(int unset_environment, const char *format, ...)
{
	g_assert(NULL != format);

	return 0;
}
