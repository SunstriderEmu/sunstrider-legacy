/* -*- C++ -*-
 * Copyright (C) 2012 Morpheus
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "Profiler.h"
#include "ProfilerMgr.h"

#if 0 //profiler disabled, fix it if you want to re use it later (ACE has been removed from the project)

Profiler::Profiler(std::string name)
{
    //printf("[TRACE] %s\n", name.c_str());
#ifdef PROFILING
  /*  _start = ACE_OS::gettimeofday();
    _fctName = name; */
#endif // PROFILING
}

Profiler::~Profiler()
{
#ifdef PROFILING
     _end = ACE_OS::gettimeofday();
    
    long elapsed = _end.usec() - _start.usec();
    if (elapsed >= 0)
        sProfilerMgr->report(_fctName, elapsed);
#endif // PROFILING
}

#endif