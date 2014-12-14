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

#ifndef _POWER_H_INCLUDED
#	define _POWER_H_INCLUDED

#	include "loginkitd-generated.h"

gboolean on_handle_can_hybrid_sleep(LoginKitManager *interface,
                                    GDBusMethodInvocation *invocation,
                                    const gchar *can,
                                    gpointer user_data);

gboolean on_handle_can_power_off(LoginKitManager *interface,
                                 GDBusMethodInvocation *invocation,
                                 const gchar *can,
                                 gpointer user_data);

gboolean on_handle_can_reboot(LoginKitManager *interface,
                              GDBusMethodInvocation *invocation,
                              const gchar *can,
                              gpointer user_data);

gboolean on_handle_can_suspend(LoginKitManager *interface,
                               GDBusMethodInvocation *invocation,
                               const gchar *can,
                               gpointer user_data);

gboolean on_handle_can_hibernate(LoginKitManager *interface,
                                 GDBusMethodInvocation *invocation,
                                 const gchar *can,
                                 gpointer user_data);

gboolean on_handle_inhibit(LoginKitManager *interface,
                           GDBusMethodInvocation *invocation,
                           const gchar *arg_what,
                           const gchar *arg_who,
                           const gchar *arg_why,
                           const gchar *arg_mode,
                           GVariant **fd,
                           gpointer user_data);

gboolean on_handle_suspend(LoginKitManager *interface,
                           GDBusMethodInvocation *invocation,
                           const gboolean arg_interactive,
                           gpointer user_data);

gboolean on_handle_hybrid_sleep(LoginKitManager *interface,
                                GDBusMethodInvocation *invocation,
                                const gboolean arg_interactive,
                                gpointer user_data);

gboolean on_handle_power_off(LoginKitManager *interface,
                             GDBusMethodInvocation *invocation,
                             const gboolean arg_interactive,
                             gpointer user_data);

gboolean on_handle_reboot(LoginKitManager *interface,
                          GDBusMethodInvocation *invocation,
                          const gboolean arg_interactive,
                          gpointer user_data);

gboolean on_handle_hibernate(LoginKitManager *interface,
                             GDBusMethodInvocation *invocation,
                             const gboolean arg_interactive,
                             gpointer user_data);

#endif