/*
* Copyright (C) 2008-2010 Trinity <http://www.trinitycore.org/>
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#ifndef TRINITY_CREATURE_TEXT_MGR_H
#define TRINITY_CREATURE_TEXT_MGR_H

#include "Creature.h"
#include "SharedDefines.h"

struct CreatureTextEntry
{
    uint32 entry;
    uint8 group;
    uint8 id;
    std::string text_en;
    std::string text_fr;
    ChatType type;
    Language lang;
    float probability;
    Emote emote;
    uint32 duration;
    uint32 sound;
};

enum TextRange
{
    TEXT_RANGE_NORMAL   = 0,
    TEXT_RANGE_AREA     = 1,
    TEXT_RANGE_ZONE     = 2,
    TEXT_RANGE_MAP      = 3,
    TEXT_RANGE_WORLD    = 4
};

typedef std::vector<CreatureTextEntry> CreatureTextGroup;              //texts in a group
typedef std::unordered_map<uint8, CreatureTextGroup> CreatureTextHolder;    //groups for a creature by groupid
typedef std::unordered_map<uint32, CreatureTextHolder> CreatureTextMap;     //all creatures by entry

//used for handling non-repeatable random texts
typedef std::vector<uint8> CreatureTextRepeatIds;
typedef std::unordered_map<uint8, CreatureTextRepeatIds> CreatureTextRepeatGroup;
typedef std::unordered_map<uint64, CreatureTextRepeatGroup> CreatureTextRepeatMap;//guid based

class CreatureTextMgr
{
    //friend class ACE_Singleton<CreatureTextMgr, ACE_Null_Mutex>;
    public:
        CreatureTextMgr() {};
        ~CreatureTextMgr() {};
        void LoadCreatureTexts();
        CreatureTextMap const& GetTextMap() const { return mTextMap; }
        
        void SendSound(Creature* source,uint32 sound, ChatType msgtype, uint64 whisperGuid, TextRange range, Team team, bool gmOnly);
        void SendEmote(Creature* source, uint32 emote);
        
        //if sent, returns the 'duration' of the text else 0 if error
        uint32 SendChat(Creature* source, uint8 textGroup, uint64 whisperGuid = 0, ChatType msgtype = CHAT_TYPE_END, Language language = LANG_ADDON, TextRange range = TEXT_RANGE_NORMAL, uint32 sound = 0, Team team = TEAM_OTHER, bool gmOnly = false, Player* srcPlr = NULL);
        void SendChatString(WorldObject* source, char const* text_en, char const* text_fr, ChatType msgtype = CHAT_TYPE_SAY, Language language = LANG_UNIVERSAL, uint64 whisperGuid = 0, TextRange range = TEXT_RANGE_NORMAL, Team team = TEAM_OTHER, bool gmOnly = false) const;
        bool TextExist(uint32 sourceEntry, uint8 textGroup);

    private:
        CreatureTextRepeatIds GetRepeatGroup(Creature* source, uint8 textGroup);
        void SetRepeatId(Creature* source, uint8 textGroup, uint8 id);

        void BuildMonsterChat(WorldPacket* data, WorldObject* source, ChatType msgtype, char const* text, Language language, uint64 whisperGuid) const;
        void SendChatPacket(WorldPacket* data_en, WorldPacket* data_fr , WorldObject* source, ChatType msgtype, uint64 whisperGuid, TextRange range, Team team, bool gmOnly) const;
        void SendMessageToSetInRange(WorldObject* source, WorldPacket* msg_en, WorldPacket* msg_fr, float dist) const;
        
        CreatureTextMap mTextMap;
        CreatureTextRepeatMap mTextRepeatMap;        
};

#define sCreatureTextMgr Trinity::Singleton<CreatureTextMgr>::Instance()
#endif
