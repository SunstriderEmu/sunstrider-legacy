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

#ifndef PROFILERMGR_H
#define	PROFILERMGR_H

#include "Common.h"
#include "Policies/SingletonImp.h"
#include "Profiler.h"

struct ProfileData
{
    uint32 count;
    long min;
    long max;
    float avg;
};

typedef std::map<std::string, ProfileData> ProfileDataMap;

class ProfilerMgr
{
    
public:
    
    friend class Trinity::Singleton<ProfilerMgr>;
    friend class Trinity::OperatorNew<ProfilerMgr>;
    
    ~ProfilerMgr();
    
    void report(std::string name, long time);
    std::string dump();
    void clear() { _data.clear(); };
    
private:

    ProfilerMgr();
    
    ProfileDataMap _data;
};

#if defined PROFILING
#define PROFILE { \
    Profiler profiler(__PRETTY_FUNCTION__); \
                }
#else
#define PROFILE NULL
#endif

#define sProfilerMgr Trinity::Singleton<ProfilerMgr>::Instance()

#endif	/* PROFILERMGR_H */

