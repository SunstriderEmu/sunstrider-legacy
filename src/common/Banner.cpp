/*
 * Copyright (C) 2008-2017 TrinityCore <http://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Banner.h"
#include "GitRevision.h"
#include "StringFormat.h"

void Trinity::Banner::Show(char const* applicationName, void(*log)(char const* text), void(*logExtraInfo)())
{
    log(Trinity::StringFormat("%s (%s)", GitRevision::GetFullVersion(), applicationName).c_str());
    log("<Ctrl-C> to stop.\n");
    log("  ____                          _            _       _               ");
    log(" / ___|   _   _   _ __    ___  | |_   _ __  (_)   __| |   ___   _ __ ");
    log(" \\___ \\  | | | | | '_ \\  / __| | __| | '__| | |  / _` |  / _ \\ | '__|");
    log("  ___) | | |_| | | | | | \\__ \\ | |_  | |    | | | (_| | |  __/ | |   ");
    log(" |____/   \\__,_| |_| |_| |___/  \\__| |_|    |_|  \\__,_|  \\___| |_|   ");
    log(" ");

    if (logExtraInfo)
        logExtraInfo();
}
