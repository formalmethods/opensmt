/*********************************************************************
Author: Roberto Bruttomesso <roberto.bruttomesso@unisi.ch>

OpenSMT -- Copyright (C) 2008, Roberto Bruttomesso

OpenSMT is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

OpenSMT is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with OpenSMT. If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/

#ifndef GLOBAL_H
#define GLOBAL_H

#define VERSION "0.1"

#include <cassert>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <list>
#include <sstream>
#include <iostream>
#include <fstream>
#include <ext/hash_map>
#include <ext/hash_set>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <stdint.h>

using std::set;
using std::map;
using std::vector;
using std::string;
using std::pair;
using std::make_pair;
using std::list;

using __gnu_cxx::hash_map;
using __gnu_cxx::hash_set;
using __gnu_cxx::hash;

using std::cout;
using std::cerr;
using std::endl;
using std::ostream;
using std::stringstream;
using std::ofstream;

#define Real double

// Set the bit B to 1 and leaves the others to 0
#define SETBIT( B ) ( 1 << (B) )

typedef enum 
{ 
    UNDEF         // Undefined logic
  , EMPTY         // Empty, for the template solver
  , QF_UF         // Uninterpreted Functions
  , QF_RDL        // Real difference logics
  , QF_IDL        // Integer difference logics
  , QF_LRA        // Real linear arithmetic
  , QF_LIA        // Integer linear arithmetic
  , QF_UFRDL      // UF + RDL
  , QF_UFIDL      // UF + IDL
  , QF_UFLRA      // UF + LRA
  , QF_UFLIA      // UF + LIA
} logic_t;

static const char * logicStr ( logic_t l )
{
       if ( EMPTY ) return "EMPTY";
  else if ( QF_UF ) return "QF_UF";
  else if ( QF_RDL ) return "QF_RDL";
  else if ( QF_IDL ) return "QF_IDL";
  else if ( QF_LRA ) return "QF_LRA";
  else if ( QF_LIA ) return "QF_LIA";
  else if ( QF_UFRDL ) return "QF_UFRDL";
  else if ( QF_UFIDL ) return "QF_UFIDL";
  else if ( QF_UFLRA ) return "QF_UFLRA";
  else if ( QF_UFLIA ) return "QF_UFLIA";
  return "";
}

static inline double cpuTime(void) 
{
    struct rusage ru;
    getrusage(RUSAGE_SELF, &ru);
    return (double)ru.ru_utime.tv_sec + (double)ru.ru_utime.tv_usec / 1000000; 
}

#if defined(__linux__)
static inline int memReadStat(int field)
{
    char    name[256];
    pid_t pid = getpid();
    sprintf(name, "/proc/%d/statm", pid);
    FILE*   in = fopen(name, "rb");
    if (in == NULL) return 0;
    int     value;
    for (; field >= 0; field--)
        fscanf(in, "%d", &value);
    fclose(in);
    return value;
}

static inline uint64_t memUsed() { return (uint64_t)memReadStat(0) * (uint64_t)getpagesize(); }

#else // stub to support every platform
static inline uint64_t memUsed() {return 0; }
#endif

#endif
