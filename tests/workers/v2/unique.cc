/*  vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 * 
 *  Gearmand client and server library.
 *
 *  Copyright (C) 2012 Data Differential, http://datadifferential.com/
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

#include <gear_config.h>
#include <libtest/test.hpp>

#include <libgearman-1.0/gearman.h>

#include "tests/workers/v2/unique.h"

#include <cassert>
#include <cstring>

// payload is unique value
gearman_return_t unique_worker_v2(gearman_job_st *job, void *)
{
  const char *workload= static_cast<const char *>(gearman_job_workload(job));

  assert(job->assigned.command == GEARMAN_COMMAND_JOB_ASSIGN_UNIQ);
  assert(gearman_job_unique(job));
  assert(strlen(gearman_job_unique(job)));
  assert(gearman_job_workload_size(job));
  assert(strlen(gearman_job_unique(job)) == gearman_job_workload_size(job));
  assert(not memcmp(workload, gearman_job_unique(job), gearman_job_workload_size(job)));
  if (gearman_job_workload_size(job) == strlen(gearman_job_unique(job)))
  {
    if (not memcmp(workload, gearman_job_unique(job), gearman_job_workload_size(job)))
    {
      if (gearman_failed(gearman_job_send_data(job, workload, gearman_job_workload_size(job))))
      {
        return GEARMAN_ERROR;
      }

      return GEARMAN_SUCCESS;
    }
  }

  return GEARMAN_FAIL;
}

