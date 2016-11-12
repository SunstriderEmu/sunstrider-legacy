
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

enum RecupProfessionType
{
    RECUP_PROFESSION_FIRST_AID      = 0,
    RECUP_PROFESSION_COOKING        = 1,
    RECUP_PROFESSION_FISHING        = 2,

    RECUP_PROFESSION_ENCHANTMENT    = 3,
    RECUP_PROFESSION_MINING         = 4,
    RECUP_PROFESSION_FORGE          = 5,
    RECUP_PROFESSION_ENGINEERING    = 6,
    RECUP_PROFESSION_LEATHERWORKING = 7,
    RECUP_PROFESSION_SKINNING       = 8,
    RECUP_PROFESSION_ALCHEMY        = 9,
    RECUP_PROFESSION_TAILORING      = 10,
    RECUP_PROFESSION_JEWELCRAFTING  = 11,
    RECUP_PROFESSION_HERBALISM      = 12,

    RECUP_PROFESSION_LOCKPICKING    = 13,
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
        static bool RecupProfession(Player* player, RecupProfessionType profession);
};
#endif
