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
 * @brief Gearmand Thread Definitions
 */

#include "gear_config.h"
#include "libgearman-server/common.h"
#include <libgearman-server/gearmand.h>

#include <cassert>
#include <cerrno>
#include <memory>

#include <libgearman-server/list.h>

/*
 * Private declarations
 */

/**
 * @addtogroup gearmand_thread_private Private Gearmand Thread Functions
 * @ingroup gearmand_thread
 * @{
 */

static void *_thread(void *data);
static void _log(const char *line, gearmand_verbose_t verbose, gearmand_thread_st *dthread);
static void _run(gearman_server_thread_st *thread, void *fn_arg);

static gearmand_error_t _wakeup_init(gearmand_thread_st *thread);
static void _wakeup_close(gearmand_thread_st *thread);
static void _wakeup_clear(gearmand_thread_st *thread);
static void _wakeup_event(int fd, short events, void *arg);
static void _clear_events(gearmand_thread_st *thread);

/** @} */

/*
 * Public definitions
 */

gearmand_error_t gearmand_thread_create(gearmand_st *gearmand)
{
  gearmand_thread_st* thread= new (std::nothrow) gearmand_thread_st;
  if (thread == NULL)
  {
    return gearmand_merror("new", gearmand_thread_st, 1);
  }

  if (! gearman_server_thread_init(gearmand_server(gearmand), &(thread->server_thread),
                                   _log, thread, gearmand_connection_watch))
  {
    delete thread;
    gearmand_fatal("gearman_server_thread_init(NULL)");
    return GEARMAN_MEMORY_ALLOCATION_FAILURE;
  }

  thread->is_thread_lock= false;
  thread->is_wakeup_event= false;
  thread->count= 0;
  thread->dcon_count= 0;
  thread->dcon_add_count= 0;
  thread->free_dcon_count= 0;
  thread->wakeup_fd[0]= -1;
  thread->wakeup_fd[1]= -1;

  gearmand_thread_list_add(thread);

  thread->dcon_list= NULL;
  thread->dcon_add_list= NULL;
  thread->free_dcon_list= NULL;

  /* If we have no threads, we still create a fake thread that uses the main
     libevent instance. Otherwise create a libevent instance for each thread. */
  if (gearmand->threads == 0)
  {
    thread->base= gearmand->base;
  }
  else
  {
    gearmand_debug("Initializing libevent for IO thread");

    thread->base= static_cast<struct event_base *>(event_base_new());
    if (thread->base == NULL)
    {
      gearmand_thread_free(thread);
      gearmand_fatal("event_base_new(NULL)");
      return GEARMAN_EVENT;
    }
  }

  gearmand_error_t ret= _wakeup_init(thread);
  if (ret != GEARMAN_SUCCESS)
  {
    gearmand_thread_free(thread);
    return ret;
  }

  /* If we are not running multi-threaded, just return the thread context. */
  if (gearmand->threads == 0)
  {
    return GEARMAN_SUCCESS;
  }

  thread->count= gearmand->thread_count;

  int pthread_ret;
  pthread_ret= pthread_mutex_init(&(thread->lock), NULL);
  if (pthread_ret != 0)
  {
    thread->count= 0;
    gearmand_thread_free(thread);

    errno= pthread_ret;
    gearmand_fatal_perror("pthread_mutex_init");
    return GEARMAN_ERRNO;
  }

  thread->is_thread_lock= true;

  gearman_server_thread_set_run(&(thread->server_thread), _run, thread);

  pthread_ret= pthread_create(&(thread->id), NULL, _thread, thread);
  if (pthread_ret != 0)
  {
    thread->count= 0;
    gearmand_thread_free(thread);

    errno= pthread_ret;
    gearmand_perror("pthread_create");

    return GEARMAN_ERRNO;
  }

  gearmand_log_debug(GEARMAN_DEFAULT_LOG_PARAM, "Thread %u created", thread->count);

  return GEARMAN_SUCCESS;
}

void gearmand_thread_free(gearmand_thread_st *thread)
{
  if (Gearmand()->threads && thread->count > 0)
  {
    gearmand_log_debug(GEARMAN_DEFAULT_LOG_PARAM, "Shutting down thread %u", thread->count);

    gearmand_thread_wakeup(thread, GEARMAND_WAKEUP_SHUTDOWN);
    int pthread_error;
    if ((pthread_error= pthread_join(thread->id, NULL)))
    {
      errno= pthread_error;
      gearmand_perror("pthread_join");
    }
  }

  if (thread->is_thread_lock)
  {
    int pthread_error;
    if ((pthread_error= pthread_mutex_destroy(&(thread->lock))))
    {
      errno= pthread_error;
      gearmand_perror("pthread_mutex_destroy");
    }
  }

  _wakeup_close(thread);

  while (thread->dcon_list != NULL)
  {
    gearmand_con_free(thread->dcon_list);
  }

  while (thread->dcon_add_list != NULL)
  {
    gearmand_con_st* dcon= thread->dcon_add_list;
    thread->dcon_add_list= dcon->next;
    gearmand_sockfd_close(dcon->fd);
    delete dcon;
  }

  while (thread->free_dcon_list != NULL)
  {
    gearmand_con_st* dcon= thread->free_dcon_list;
    thread->free_dcon_list= dcon->next;
    delete dcon;
  }

  gearman_server_thread_free(&(thread->server_thread));

  gearmand_thread_list_free(thread);

  if (Gearmand()->threads > 0)
  {
    if (thread->base != NULL)
    {
      event_base_free(thread->base);
      thread->base= NULL;
    }

    gearmand_log_debug(GEARMAN_DEFAULT_LOG_PARAM, "Thread %u shutdown complete", thread->count);
  }

  delete thread;
}

void gearmand_thread_wakeup(gearmand_thread_st *thread,
                            gearmand_wakeup_t wakeup)
{
  uint8_t buffer= wakeup;

  /* If this fails, there is not much we can really do. This should never fail
     though if the thread is still active. */
  if (write(thread->wakeup_fd[1], &buffer, 1) != 1)
  {
    gearmand_perror("write");
  }
}

void gearmand_thread_run(gearmand_thread_st *thread)
{
  while (1)
  {
    gearmand_error_t ret;
    gearmand_con_st *dcon= gearman_server_thread_run(&(thread->server_thread), &ret);

    if (ret == GEARMAN_SUCCESS or
        ret == GEARMAN_IO_WAIT or
        ret == GEARMAN_SHUTDOWN_GRACEFUL)
    {
      return;
    }

    if (dcon == NULL)
    {
      /* We either got a GEARMAN_SHUTDOWN or some other fatal internal error.
         Either way, we want to shut the server down. */
      gearmand_wakeup(Gearmand(), GEARMAND_WAKEUP_SHUTDOWN);
      return;
    }

    gearmand_log_info(GEARMAN_DEFAULT_LOG_PARAM, "Disconnected %s:%s", dcon->host, dcon->port);

    gearmand_con_free(dcon);
  }
}

#ifndef __INTEL_COMPILER
#pragma GCC diagnostic ignored "-Wold-style-cast"
#endif

/*
 * Private definitions
 */

static void *_thread(void *data)
{
  gearmand_thread_st *thread= (gearmand_thread_st *)data;
  char buffer[BUFSIZ];

  int length= snprintf(buffer, sizeof(buffer), "[%6u ]", thread->count);
  if (length <= 0 or sizeof(length) >= sizeof(buffer))
  {
    assert(0);
    buffer[0]= 0;
  }

  gearmand_initialize_thread_logging(buffer);

  gearmand_debug("Entering thread event loop");

  if (event_base_loop(thread->base, 0) == -1)
  {
    gearmand_fatal("event_base_loop(-1)");
    Gearmand()->ret= GEARMAN_EVENT;
  }

  gearmand_debug("Exiting thread event loop");

  return NULL;
}

static void _log(const char *line, gearmand_verbose_t verbose, gearmand_thread_st *dthread)
{
  (void)dthread;
  (*Gearmand()->log_fn)(line, verbose, (void *)Gearmand()->log_context);
}

static void _run(gearman_server_thread_st *thread __attribute__ ((unused)),
                 void *fn_arg)
{
  gearmand_thread_st *dthread= (gearmand_thread_st *)fn_arg;
  gearmand_thread_wakeup(dthread, GEARMAND_WAKEUP_RUN);
}

static gearmand_error_t _wakeup_init(gearmand_thread_st *thread)
{
  int ret;

  gearmand_debug("Creating IO thread wakeup pipe");

  ret= pipe(thread->wakeup_fd);
  if (ret == -1)
  {
    gearmand_perror("pipe");
    return GEARMAN_ERRNO;
  }

  ret= fcntl(thread->wakeup_fd[0], F_GETFL, 0);
  if (ret == -1)
  {
    gearmand_perror("fcntl(F_GETFL)");
    return GEARMAN_ERRNO;
  }

  ret= fcntl(thread->wakeup_fd[0], F_SETFL, ret | O_NONBLOCK);
  if (ret == -1)
  {
    gearmand_perror("fcntl(F_SETFL)");
    return GEARMAN_ERRNO;
  }

  event_set(&(thread->wakeup_event), thread->wakeup_fd[0], EV_READ | EV_PERSIST,
            _wakeup_event, thread);
  event_base_set(thread->base, &(thread->wakeup_event));

  if (event_add(&(thread->wakeup_event), NULL) < 0)
  {
    gearmand_perror("event_add");
    return GEARMAN_EVENT;
  }

  thread->is_wakeup_event= true;

  return GEARMAN_SUCCESS;
}

static void _wakeup_close(gearmand_thread_st *thread)
{
  _wakeup_clear(thread);

  if (thread->wakeup_fd[0] >= 0)
  {
    gearmand_debug("Closing IO thread wakeup pipe");
    gearmand_pipe_close(thread->wakeup_fd[0]);
    thread->wakeup_fd[0]= -1;
    gearmand_pipe_close(thread->wakeup_fd[1]);
    thread->wakeup_fd[1]= -1;
  }
}

static void _wakeup_clear(gearmand_thread_st *thread)
{
  if (thread->is_wakeup_event)
  {
    gearmand_log_debug(GEARMAN_DEFAULT_LOG_PARAM, "Clearing event for IO thread wakeup pipe %u", thread->count);
    if (event_del(&(thread->wakeup_event)) < 0)
    {
      gearmand_perror("event_del");
      assert(! "event_del");
    }
    thread->is_wakeup_event= false;
  }
}

static void _wakeup_event(int fd, short events __attribute__ ((unused)), void *arg)
{
  gearmand_thread_st *thread= (gearmand_thread_st *)arg;
  uint8_t buffer[GEARMAN_PIPE_BUFFER_SIZE];
  ssize_t ret;

  while (1)
  {
    ret= read(fd, buffer, GEARMAN_PIPE_BUFFER_SIZE);
    if (ret == 0)
    {
      _clear_events(thread);
      gearmand_fatal("read(EOF)");
      Gearmand()->ret= GEARMAN_PIPE_EOF;
      return;
    }
    else if (ret == -1)
    {
      if (errno == EINTR)
        continue;

      if (errno == EAGAIN)
        break;

      _clear_events(thread);
      gearmand_perror("_wakeup_event:read");
      Gearmand()->ret= GEARMAN_ERRNO;
      return;
    }

    for (ssize_t x= 0; x < ret; x++)
    {
      switch ((gearmand_wakeup_t)buffer[x])
      {
      case GEARMAND_WAKEUP_PAUSE:
        gearmand_debug("Received PAUSE wakeup event");
        break;

      case GEARMAND_WAKEUP_SHUTDOWN_GRACEFUL:
        gearmand_debug("Received SHUTDOWN_GRACEFUL wakeup event");
        if (gearman_server_shutdown_graceful(&(Gearmand()->server)) == GEARMAN_SHUTDOWN)
        {
          gearmand_wakeup(Gearmand(), GEARMAND_WAKEUP_SHUTDOWN);
        }
        break;

      case GEARMAND_WAKEUP_SHUTDOWN:
        gearmand_debug("Received SHUTDOWN wakeup event");
        _clear_events(thread);
        break;

      case GEARMAND_WAKEUP_CON:
        gearmand_debug("Received CON wakeup event");
        gearmand_con_check_queue(thread);
        break;

      case GEARMAND_WAKEUP_RUN:
        gearmand_debug("Received RUN wakeup event");
        gearmand_thread_run(thread);
        break;

      default:
        gearmand_log_fatal(GEARMAN_DEFAULT_LOG_PARAM, "Received unknown wakeup event (%u)", buffer[x]);
        _clear_events(thread);
        Gearmand()->ret= GEARMAN_UNKNOWN_STATE;
        break;
      }
    }
  }
}

static void _clear_events(gearmand_thread_st *thread)
{
  _wakeup_clear(thread);

  while (thread->dcon_list != NULL)
  {
    gearmand_con_free(thread->dcon_list);
  }
}
