/*ckwg +29
 * Copyright 2016-2017 by Kitware, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 *  * Neither name of Kitware, Inc. nor the names of any contributors may be used
 *    to endorse or promote products derived from this software without specific
 *    prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef KWIVER_VITAL_UTIL_STRING_FORMAT_H
#define KWIVER_VITAL_UTIL_STRING_FORMAT_H

#include <stdarg.h>  // For va_start, etc.
#include <memory>    // For std::unique_ptr
#include <string>
#include <cstring>
#include <sstream>
#include <algorithm>
#include <iterator>

namespace kwiver {
namespace vital {

/**
 * @brief Printf style formatting for std::string
 *
 * This function creates a std::string from an printf style input
 * format specifier and a list of values.
 *
 * @param fmt_str Formatting string using embedded printf format specifiers.
 *
 * @return Formatted string.
 */
inline std::string
string_format( const std::string fmt_str, ... )
{
  int final_n, n = ( (int)fmt_str.size() ) * 2; /* Reserve two times as much as the length of the fmt_str */
  std::string str;
  std::unique_ptr< char[] > formatted;
  va_list ap;

  while ( 1 )
  {
    formatted.reset( new char[n] );   /* Wrap the plain char array into the unique_ptr */
    strcpy( &formatted[0], fmt_str.c_str() );
    va_start( ap, fmt_str );
    final_n = vsnprintf( &formatted[0], n, fmt_str.c_str(), ap );
    va_end( ap );
    if ( ( final_n < 0 ) || ( final_n >= n ) )
    {
      n += abs( final_n - n + 1 );
    }
    else
    {
      break;
    }
  }

  return std::string( formatted.get() );
}


/**
 * @brief Does string start with pattern
 *
 * This function checks to see if the input starts with the supplied
 * pattern.
 *
 * @param input String to be checked
 * @param pattern String to use for checking.
 *
 * @return \b true if string starts with pattern
 */
inline bool
starts_with( const std::string& input, const std::string& pattern)
{
  return (0 == input.compare( 0, pattern.size(), pattern ) );
}


/**
 * @brief Join a set of strings with specified separator.
 *
 * A single string is created and returned from the supplied vector of
 * strings with the specified separator inserted between
 * strings. There is no trailing separator.
 *
 * @param elements Vector of elements to join
 * @param str_separator String to be placed between elements
 *
 * @return Single string with all elements joined with separator.
 */
inline std::string
join( const std::vector<std::string>& elements, const std::string& str_separator)
{
  const char* const separator = str_separator.c_str();
  switch (elements.size())
  {
  case 0:
    return "";

  case 1:
    return elements[0];

  default:
  {
    std::ostringstream os;
    std::copy(elements.begin(), elements.end()-1,
              std::ostream_iterator<std::string>(os, separator));
    os << *elements.rbegin();
    return os.str();
  }
  } // end switch
}

} } // end namespace

#endif /* KWIVER_VITAL_UTIL_STRING_FORMAT_H */
