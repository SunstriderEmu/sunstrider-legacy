
#ifndef TRINITYCORE_CHANNELMGR_H
#define TRINITYCORE_CHANNELMGR_H

#include "Channel.h"

#include <map>
#include <string>

class ChannelMgr
{
    public:
        typedef std::map<std::string,Channel *> ChannelMap;
        ChannelMgr() {}
        ~ChannelMgr()
        {
            for(auto & channel : channels)
                delete channel.second;
            channels.clear();
        }
        Channel *GetJoinChannel(const std::string& name, uint32 channel_id)
        {
            if(channels.count(name) == 0)
            {
                auto nchan = new Channel(name,channel_id);
                channels[name] = nchan;
            }
            return channels[name];
        }
        Channel *GetChannel(const std::string& name, Player *p)
        {
            ChannelMap::const_iterator i = channels.find(name);

            if(i == channels.end())
            {
                WorldPacket data;
                MakeNotOnPacket(&data,name);
                p->SendDirectMessage(&data);
                return nullptr;
            }
            else
                return i->second;
        }
        Channel *GetChannel(const std::string& name)
        {
            ChannelMap::const_iterator i = channels.find(name);
            
            if (i != channels.end())
                return i->second;
                
            return nullptr;
        }
        void LeftChannel(const std::string& name)
        {
            ChannelMap::const_iterator i = channels.find(name);

            if(i == channels.end())
                return;

            Channel* channel = i->second;

            if(channel->IsEmpty() && !channel->IsConstant())
            {
                channels.erase(name);
                delete channel;
            }
        }
    private:
        ChannelMap channels;
        void MakeNotOnPacket(WorldPacket *data, const std::string& name)
        {
            data->Initialize(SMSG_CHANNEL_NOTIFY, (1+10));  // we guess size
            (*data) << (uint8)0x05 << name;
        }
};

inline ChannelMgr* channelMgr(uint32 team)
{
    static ChannelMgr allianceChannelMgr;
    static ChannelMgr hordeChannelMgr;
    
    if (sWorld->getConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_CHANNEL))
                                                            //For Test,No Seprate Faction
        return &allianceChannelMgr;

    if(team == TEAM_ALLIANCE)
        return &allianceChannelMgr;
    if(team == TEAM_HORDE)
        return &hordeChannelMgr;
    return nullptr;
}
#endif

