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


#pragma once

#include <string>
#include <unistd.h>

class Context
{
public:
  gearman_worker_st *worker;
  in_port_t _port;
  std::string _worker_function_name;
  server_startup_st &_servers;
  bool run_worker;
  std::vector<std::string> _extra_files;

  Context(in_port_t port_arg, server_startup_st &server_arg):
    worker(NULL),
    _port(port_arg),
    _worker_function_name("queue_test"),
    _servers(server_arg),
    run_worker(false)
  {
  }

  ~Context()
  {
    extra_clear();
    _servers.clear();
  }

  const char *worker_function_name() const
  {
    return _worker_function_name.c_str();
  }

  in_port_t port() const
  {
    return _port;
  }

  bool initialize(int argc, const char *argv[])
  {
    if (server_startup(_servers, "gearmand", _port, argc, argv) == false)
    {
      return false;
    }

    if ((worker= gearman_worker_create(NULL)) == NULL)
    {
      return false;
    }

    if (gearman_failed(gearman_worker_add_server(worker, NULL, _port)))
    {
      return false;
    }

    return true;
  }

  void extra_file(const std::string& extra_file_)
  {
    _extra_files.push_back(extra_file_);
  }

  void extra_clear()
  {
    for (std::vector<std::string>::iterator iter= _extra_files.begin();
         iter != _extra_files.end();
         ++iter)
    {
      unlink((*iter).c_str());
    }
    _extra_files.clear();
  }

  void reset()
  {
    _servers.clear();
    gearman_worker_free(worker);

    worker= NULL;
    extra_clear();
  }
};
