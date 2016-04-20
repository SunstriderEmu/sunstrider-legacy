
#ifndef TRINITY_RECUPMGR_H
#define TRINITY_RECUPMGR_H

class ChatHandler;

enum RecupEquipmentType
{
    RECUP_EQUIP_TYPE_DPS          = 0,
    RECUP_EQUIP_TYPE_HEAL         = 1,
    RECUP_EQUIP_TYPE_TANK         = 2,
    RECUP_EQUIP_TYPE_DPS_ALT      = 3
};

enum RecupStuffLevel
{
    RECUP_STUFF_LEVEL_0 = 0,
};

class RecupMgr
{
    public:
		static bool HandleRecupParseCommand(Player *player, std::string command, uint32 metier_level, bool equip = false, ChatHandler* handler = nullptr);
        static bool Recup(Player* player, RecupEquipmentType type, RecupStuffLevel level);
};
#endif
