/*  vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 * 
 *  Gearmand client and server library.
 *
 *  Copyright (C) 2011-2012 Data Differential, http://datadifferential.com/
 *  Copyright (C) 2008 Brian Aker, Eric Day
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

/**
 * @file
 * @brief Server function definitions
 */

#include "gear_config.h"
#include "libgearman-server/common.h"

#include <cstring>
#include <memory>

#include <libgearman-server/list.h>

/*
 * Public definitions
 */

static gearman_server_function_st* gearman_server_function_create(gearman_server_st *server)
{
  gearman_server_function_st* function= new (std::nothrow) gearman_server_function_st;

  if (function == NULL)
  {
    gearmand_merror("new gearman_server_function_st", gearman_server_function_st, 0);
    return NULL;
  }

  function->worker_count= 0;
  function->job_count= 0;
  function->job_total= 0;
  function->job_running= 0;
  memset(function->max_queue_size, GEARMAN_DEFAULT_MAX_QUEUE_SIZE,
         sizeof(uint32_t) * GEARMAN_JOB_PRIORITY_MAX);
  function->function_name_size= 0;
  gearmand_server_list_add(server, function);
  function->function_name= NULL;
  function->worker_list= NULL;
  memset(function->job_list, 0,
         sizeof(gearman_server_job_st *) * GEARMAN_JOB_PRIORITY_MAX);
  memset(function->job_end, 0,
         sizeof(gearman_server_job_st *) * GEARMAN_JOB_PRIORITY_MAX);

  return function;
}

#ifndef __INTEL_COMPILER
#pragma GCC diagnostic ignored "-Wold-style-cast"
#endif

gearman_server_function_st *
gearman_server_function_get(gearman_server_st *server,
                            const char *function_name,
                            size_t function_name_size)
{
  gearman_server_function_st *function;

  for (function= server->function_list; function != NULL;
       function= function->next)
  {
    if (function->function_name_size == function_name_size and
        memcmp(function->function_name, function_name, function_name_size) == 0)
    {
      return function;
    }
  }

  function= gearman_server_function_create(server);
  if (function == NULL)
  {
    return NULL;
  }

  function->function_name= new char[function_name_size +1];
  if (function->function_name == NULL)
  {
    gearmand_merror("new[]", char,  function_name_size +1);
    gearman_server_function_free(server, function);
    return NULL;
  }

  memcpy(function->function_name, function_name, function_name_size);
  function->function_name[function_name_size]= 0;
  function->function_name_size= function_name_size;

  return function;
}

void gearman_server_function_free(gearman_server_st *server, gearman_server_function_st *function)
{
  delete [] function->function_name;

  gearmand_server_list_free(server, function);

  delete function;
}
