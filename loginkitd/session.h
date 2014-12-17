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

#ifndef _LOGINKITD_SESSION_H_INCLUDED
#	define _LOGINKITD_SESSION_H_INCLUDED

#	include "loginkitd-generated.h"

gboolean on_handle_unlock_session(LoginKitManager *interface,
                                  GDBusMethodInvocation *invocation,
                                  const gchar *arg_session,
                                  gpointer user_data);

gboolean on_handle_activate_session_on_seat(LoginKitManager *interface,
                                            GDBusMethodInvocation *invocation,
                                            const gchar *arg_session,
                                            const gchar *arg_seat,
                                            gpointer user_data);

gboolean on_handle_get_session(LoginKitManager *interface,
                               GDBusMethodInvocation *invocation,
                               const gchar *arg_ssid,
                               gpointer user_data);

#endif
