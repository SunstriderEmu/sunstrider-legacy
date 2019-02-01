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
    log("<Ctrl-C> to stop.\\n");

    log("    ____                             __                   __                      ");
    log("   /\\  _`\\                          /\\ \\__         __    /\\ \\                     ");
    log("   \\ \\,\\L\\_\\  __  __    ___     ____\\ \\ ,_\\  _ __ /\\_\\   \\_\\ \\     __   _ __      ");
    log("    \\/_\\__ \\ /\\ \\/\\ \\ /' _ `\\  /',__\\\\ \\ \\/ /\\`'__\\/\\ \\  /'_` \\  /'__`\\/\\`'__\\    ");
    log("      /\\ \\L\\ \\ \\ \\_\\ \\/\\ \\/\\ \\/\\__, `\\\\ \\ \\_\\ \\ \\/ \\ \\ \\/\\ \\L\\ \\/\\  __/\\ \\ \\/     ");
    log("      \\ `\\____\\ \\____/\\ \\_\\ \\_\\/\\____/ \\ \\__\\\\ \\_\\  \\ \\_\\ \\___,_\\ \\____\\\\ \\_\\     ");
    log("       \\/_____/\\/___/  \\/_/\\/_/\\/___/   \\/__/ \\/_/   \\/_/\\/__,_ /\\/____/ \\/_/     ");
    log("                                                                                  ");
    log(" ");
                                                                       
    if (logExtraInfo)
        logExtraInfo();
}
