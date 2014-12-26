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

#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>

#include "fd.h"

__attribute__((visibility("default")))
int sd_listen_fds(int unset_environment)
{
	return 0;
}

__attribute__((visibility("default")))
int sd_is_socket(int fd, int family, int type, int listening)
{
	struct sockaddr sa;
	socklen_t salen;
	socklen_t optlen;
	int optval;

	if (AF_UNSPEC != family) {
		if (-1 == getsockname(fd, &sa, &salen))
			return -errno;

		if (family != sa.sa_family)
			return 0;
	}

	if (0 != type) {
		if (-1 == getsockopt(fd, SOL_SOCKET, SO_TYPE, &optval, &optlen))
			return -errno;

		if (type != optval)
			return 0;
	}

	if (0 <= listening) {
		if (-1 == getsockopt(fd, SOL_SOCKET, SO_ACCEPTCONN, &optval, &optlen))
			return -errno;

		/* quote from the sd_is_socket() man page: "If listening is 0, it is
		 * checked whether the socket is not in this mode." */
		if ((0 != optval) && (0 == listening))
			return 0;

		/* quote from the sd_is_socket() man page: "If the listening parameter
		 * is positive, it is checked whether the socket is in accepting mode,
		 * i.e. listen() has been called for it" */
		if ((1 != optval) && (0 < listening))
			return 0;
	}

	return 1;
}
