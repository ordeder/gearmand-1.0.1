/*  vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 *
 *  Data Differential's libhostle
 *
 *  Copyright (C) 2012 Data Differential, http://datadifferential.com/
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are
 *  met:
 *
 *      * Redistributions of source code must retain the above copyright
 *  notice, this list of conditions and the following disclaimer.
 *
 *      * Redistributions in binary form must reproduce the above
 *  copyright notice, this list of conditions and the following disclaimer
 *  in the documentation and/or other materials provided with the
 *  distribution.
 *
 *      * The names of its contributors may not be used to endorse or
 *  promote products derived from this software without specific prior
 *  written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#pragma once

#include <stdbool.h>

#include "libhostile/visibility.h"

enum hostile_poll_t
{
  HOSTILE_POLL_CLOSED,
  HOSTILE_POLL_SHUT_WR,
  HOSTILE_POLL_SHUT_RD
};

#ifndef __cplusplus
typedef enum hostile_poll_t hostile_poll_t;
#endif


#ifdef __cplusplus
extern "C" {
#endif

LIBHOSTILE_API
  bool libhostile_is_accept(void);

LIBHOSTILE_API
  void set_poll_close(bool arg, int frequency, int not_until_arg, enum hostile_poll_t poll_type);

LIBHOSTILE_API
  void set_accept_close(bool arg, int frequency, int not_until_arg);

LIBHOSTILE_API
  void set_recv_corrupt(bool arg, int frequency, int not_until_arg);

LIBHOSTILE_API
  void set_recv_close(bool arg, int frequency, int not_until_arg);

LIBHOSTILE_API
  void set_send_close(bool arg, int frequency, int not_until_arg);

LIBHOSTILE_API
  void hostile_dump(void);

#ifdef __cplusplus
}
#endif
