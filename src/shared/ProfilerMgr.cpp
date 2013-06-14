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

#include "ProfilerMgr.h"

INSTANTIATE_SINGLETON_1(ProfilerMgr);

ProfilerMgr::ProfilerMgr()
{
    _data.clear();
}

ProfilerMgr::~ProfilerMgr()
{
}

void ProfilerMgr::report(std::string name, long time)
{
    ProfileDataMap::iterator itr = _data.find(name);
    if (itr != _data.end()) { // Profiling data already existing
        if (time < itr->second.min)
            itr->second.min = time;
        
        if (time > itr->second.max)
            itr->second.max = time;
        
        itr->second.avg = ((itr->second.avg * itr->second.count) + time);
        itr->second.count++;
        itr->second.avg /= float(itr->second.count);
    }
    else { // New profiling data
        ProfileData data;
        data.count = 1;
        data.min = time;
        data.max = time;
        data.avg = float(time);
        
        _data[name] = data;
    }
}

std::string ProfilerMgr::dump()
{
    std::ostringstream out;
    
    out << "[PROFILING DATA]\n";
    for (ProfileDataMap::iterator itr = _data.begin(); itr != _data.end(); ++itr) {
        out << "\t" << itr->first << ": " << itr->second.count << " calls";
        out << " - min/max/avg = " << itr->second.min << "/" << itr->second.max << "/" << itr->second.avg << " µs\n"; 
    }
    out << "[/PROFILING DATA]";
    
    return out.str();
}