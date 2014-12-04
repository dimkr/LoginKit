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

#ifndef _LOGINKIT_H_INCLUDED
#	define _LOGINKIT_H_INCLUDED

#	include <sys/types.h>

int sd_pid_get_session(pid_t pid, char **session);
int sd_pid_get_owner_uid(pid_t pid, uid_t *uid);
int sd_pid_get_machine_name(pid_t pid, char **name);
int sd_pid_get_unit(pid_t pid, char **unit);
int sd_pid_get_user_unit(pid_t pid, char **unit);
int sd_pid_get_slice(pid_t pid, char **slice);

/*
max2344:  dimkr it will walk the process table in /proc/ and get the info from fd/ and ./stat in the source misc/ss.c around line 300 static void user_ent_hash_build(void) is the relevant part
  */

/*
 TODO:
   int sd_peer_get_machine_name(int fd, char **name);
   int sd_peer_get_session(int fd, char **session);
   int sd_peer_get_unit(int fd, char **unit);
   int sd_peer_get_user_unit(int fd, char **unit);
   int sd_peer_get_owner_uid(int fd, uid_t *uid);
   int sd_peer_get_slice(int fd, char **slice);
 */

#endif
