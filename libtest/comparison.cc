/*  vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 *
 *  Data Differential YATL (i.e. libtest)  library
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

#include "gear_config.h"
#include <libtest/common.h>

namespace libtest {

bool jenkins_is_caller(void)
{
  if (bool(getenv("JENKINS_HOME")))
  {
    return true;
  }

  return false;
}

bool valgrind_is_caller(void)
{
  if (bool(getenv("TESTS_ENVIRONMENT")) and strstr(getenv("TESTS_ENVIRONMENT"), "valgrind"))
  {
    return true;
  }

  return false;
}

bool gdb_is_caller(void)
{
  if (bool(getenv("TESTS_ENVIRONMENT")) and strstr(getenv("TESTS_ENVIRONMENT"), "gdb"))
  {
    return true;
  }

  return false;
}

bool helgrind_is_caller(void)
{
  if (bool(getenv("TESTS_ENVIRONMENT")) and strstr(getenv("TESTS_ENVIRONMENT"), "helgrind"))
  {
    return true;
  }

  return false;
}

bool _in_valgrind(const char*, int, const char*)
{
  if (valgrind_is_caller())
  {
    return true;
  }

  return false;
}

} // namespace libtest
