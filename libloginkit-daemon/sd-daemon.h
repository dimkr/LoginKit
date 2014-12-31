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

#ifndef _LOGINKIT_DAEMON_H_INCLUDED
#	define _LOGINKIT_DAEMON_H_INCLUDED

#	include <syslog.h>

#	include <libloginkit-daemon/misc.h>
#	include <libloginkit-daemon/fd.h>

#	define SD_PASTE(x) #x
#	define SD_STR(x) SD_PASTE(x)

/* the SD* string constants mirror the LOG_* integers */
#	define SD_EMERG "<" SD_STR(LOG_EMERG) ">"
#	define SD_ALERT "<" SD_STR(LOG_ALERT) ">"
#	define SD_CRIT "<" SD_STR(LOG_CRIT) ">"
#	define SD_ERR "<" SD_STR(LOG_ERR) ">"
#	define SD_WARNING "<" SD_STR(LOG_WARNING) ">"
#	define SD_NOTICE "<" SD_STR(LOG_NOTICE) ">"
#	define SD_INFO "<" SD_STR(LOG_INFO) ">"
#	define SD_DEBUG "<" SD_STR(LOG_DEBUG) ">"

#endif
