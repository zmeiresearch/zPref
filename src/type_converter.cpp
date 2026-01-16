/*==============================================================================
   zPref - NVS-backed preferences library for ESP32

   Simple template type converter to and from string

   Copyright 2020-2026 Ivan Vasilev, Zmei Research Ltd.

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
  ============================================================================*/

//==============================================================================
//  Includes
//==============================================================================
#include <sstream>
#include "WString.h"
#include "type_converter.hpp"

//==============================================================================
//  Defines
//==============================================================================
#define CMP_NAME                "TypeConvert"

//==============================================================================
//  Local types
//==============================================================================

//==============================================================================
//  Local function definitions
//==============================================================================

//==============================================================================
//  Local data
//==============================================================================

//==============================================================================
//  Local functions
//==============================================================================

//==============================================================================
//  Exported data
//==============================================================================

//==============================================================================
//  Exported functions
//==============================================================================

template<>
String getValue_as(const char * const val) {
    return String(val);
}

template<>
bool getValue_as(const char * const val) {
    return String(val).equals("true") || String(val).equals("1") || String(val).equals("True");
}

template<>
unsigned short getValue_as(const char * const val) {
    return (unsigned short) strtoul(val, NULL, 0);
}

template<>
unsigned char getValue_as(const char * const val) {
    return (unsigned char) strtoul(val, NULL, 0);
}

template<>
long long getValue_as(const char * const val) {
    return strtoll(val, NULL, 0);
}

template<typename C>
C getValue_as(const char * const val) {
    C ret;
    std::istringstream ss(val);
    ss >> ret;
    return ret;
}
