#ifndef __NPCHANDLER_H
#define __NPCHANDLER_H

// GCC have alternative #pragma pack(N) syntax and old gcc version not support pack(push,N), also any gcc version not support it at some platform
#if defined( __GNUC__ )
#pragma pack(1)
#else
#pragma pack(push,1)
#endif

// GCC have alternative #pragma pack() syntax and old gcc version not support pack(pop), also any gcc version not support it at some platform
#if defined( __GNUC__ )
#pragma pack()
#else
#pragma pack(pop)
#endif

struct QEmote
{
    uint32 _Emote;
    uint32 _Delay;
};

#define MAX_GOSSIP_TEXT_EMOTES 3

struct GossipTextOption
{
    std::string Text_0;
    std::string Text_1;
    uint32 BroadcastTextID;
    uint32 Language;
    float Probability;
    QEmote Emotes[3];
};

#define MAX_GOSSIP_TEXT_OPTIONS 8

struct GossipText
{
    uint32 Text_ID;
    GossipTextOption Options[MAX_GOSSIP_TEXT_OPTIONS];
};

struct PageTextLocale
{
    std::vector<std::string> Text;
};

struct NpcTextLocale
{
    NpcTextLocale() { Text_0.resize(MAX_GOSSIP_TEXT_OPTIONS); Text_1.resize(MAX_GOSSIP_TEXT_OPTIONS); }

    std::vector<std::vector<std::string> > Text_0;
    std::vector<std::vector<std::string> > Text_1;
};
#endif

