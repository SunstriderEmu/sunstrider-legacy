/*
 * Copyright (C) 2008-2009 Trinity <http://www.trinitycore.org/>
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
 /*
 *  TrinityIRC by Machiavelli (machiaveltman@gmail.com),
 *  based on MangChat by Cybrax (cybraxvd@gmail.com).
 *  Please give proper credit for the authors' hard work.
 */
 
#ifndef IRCCHATHANDLER
#define IRCCHATHANDLER

#include "IRC.h"
#include "../game/ObjectMgr.h"
#include "../game/Chat.h"

class IRCHandler : public ChatHandler
{
    public:
        explicit IRCHandler(const char* _nickname, ClientData const* _user) : user(_user), nickname(_nickname){}
        
        const char *GetName() const;
        const char *GetTrinityString(int32 entry) const;
        bool isAvailable(ChatCommand const& cmd) const;
        void SendSysMessage(const char *str);
        std::string GetNameLink() const;
        bool needReportToTarget(Player* chr) const;
        LocaleConstant GetSessionDbcLocale() const;
        int GetSessionDbLocaleIndex() const;

        
    protected:
        ClientData const* user;
        const char* nickname;
};

#endif
