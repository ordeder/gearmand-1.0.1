/*  vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 * 
 *  Gearmand client and server library.
 *
 *  Copyright (C) 2011 Data Differential, http://datadifferential.com/
 *  All rights reserved.
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

#include "gear_config.h"

#include <libtest/test.hpp>

using namespace libtest;

#include <cassert>
#include <cstring>
#include <libgearman/gearman.h>
#include <tests/do_background.h>

#ifndef __INTEL_COMPILER
#pragma GCC diagnostic ignored "-Wold-style-cast"
#endif

test_return_t gearman_client_do_background_basic(void *object)
{
  gearman_client_st *client= (gearman_client_st *)object;
  const char *worker_function= (const char *)gearman_client_context(client);
  char job_handle[GEARMAN_JOB_HANDLE_SIZE]= {0};

  gearman_return_t rc= gearman_client_do_background(client,
                                                    worker_function,
                                                    NULL,
                                                    test_literal_param("foobar"),
                                                    job_handle);
  test_compare(GEARMAN_SUCCESS, rc);
  test_truth(job_handle[0]);

  return TEST_SUCCESS;
}

test_return_t gearman_client_do_high_background_basic(void *object)
{
  gearman_client_st *client= (gearman_client_st *)object;
  const char *worker_function= (const char *)gearman_client_context(client);
  char job_handle[GEARMAN_JOB_HANDLE_SIZE]= {0};

  gearman_return_t rc= gearman_client_do_high_background(client,
                                                         worker_function,
                                                         NULL,
                                                         test_literal_param("foobar"),
                                                         job_handle);
  test_compare(GEARMAN_SUCCESS, rc);
  test_truth(job_handle[0]);

  return TEST_SUCCESS;
}

test_return_t gearman_client_do_low_background_basic(void *object)
{
  gearman_client_st *client= (gearman_client_st *)object;
  const char *worker_function= (const char *)gearman_client_context(client);
  char job_handle[GEARMAN_JOB_HANDLE_SIZE]= {0};

  gearman_return_t rc= gearman_client_do_low_background(client,
                                                         worker_function,
                                                         NULL,
                                                         test_literal_param("foobar"),
                                                         job_handle);

  test_compare(GEARMAN_SUCCESS, rc);
  test_truth(job_handle[0]);

  return TEST_SUCCESS;
}
