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

static std::string printer(const char *str, size_t length)
{
  std::ostringstream buf;
  for (size_t x= 0; x < length; x++)
  {
    if (isprint(str[x]))
    {
      buf << str[x];
    }
    else
    {
      buf << "(" << int(str[x]) << ")";
    }
  }

  return buf.str();
}

namespace vchar {

int compare(libtest::vchar_t& arg, const char *str, size_t length)
{
  if (arg.size() == length and (memcmp(&arg[0], str, length) == 0))
  {
    return 0;
  }
  else if (arg.size() > length)
  {
    return 1;
  }

  return -1;
}

void make(libtest::vchar_t& arg)
{
  size_t length= rand() % 1024;
  make(arg, length);
}

void make(libtest::vchar_t& arg, size_t length)
{
  arg.reserve(length);
  for (uint32_t x= 0; x < length; x++)
  {
    arg.push_back(char(x % 127));
  }
}

} // namespace vchar

void make_vector(libtest::vchar_t& arg, const char *str, size_t length)
{
  arg.resize(length);
  memcpy(&arg[0], str, length);
}

std::ostream& operator<<(std::ostream& output, const libtest::vchar_t& arg)
{
  std::string tmp= libtest::printer(&arg[0], arg.size());
  output << tmp <<  "[" << arg.size() << "]";

  return output;
}

} // namespace libtest
