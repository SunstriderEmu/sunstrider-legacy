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

#ifndef PROFILER_H
#define    PROFILER_H

#include "Common.h"
#include <ace/Time_Value.h>

class Profiler
{
    
public:
    
    Profiler(std::string name);
    ~Profiler();
    
private:
    
    ACE_Time_Value _start;
    ACE_Time_Value _end;
    std::string _fctName;
};

#endif    /* PROFILER_H */

