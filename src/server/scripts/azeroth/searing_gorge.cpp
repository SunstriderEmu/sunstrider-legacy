/* Copyright (C) 2006 - 2008 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
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

/* ScriptData
SDName: Searing_Gorge
SD%Complete: 80
SDComment: Quest support: 3377, 3441 (More accurate info on Kalaran needed). Lothos Riftwaker teleport to Molten Core.
SDCategory: Searing Gorge
EndScriptData */

/* ContentData
npc_kalaran_windblade
npc_lothos_riftwaker
npc_zamael_lunthistle
EndContentData */



/*######
## npc_kalaran_windblade
######*/

bool GossipHello_npc_kalaran_windblade(Player *player, Creature *_Creature)
{
    if (_Creature->IsQuestGiver())
        player->PrepareQuestMenu( _Creature->GetGUID() );

    //todo translate
    if (player->GetQuestStatus(3441) == QUEST_STATUS_INCOMPLETE)
        player->ADD_GOSSIP_ITEM( GOSSIP_ICON_CHAT, "Expliquez-moi ce qui dirige votre vengeance ?", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);

    SEND_PREPARED_GOSSIP_MENU(player, _Creature);

    return true;
}

bool GossipSelect_npc_kalaran_windblade(Player *player, Creature *_Creature, uint32 sender, uint32 action)
{
    switch (action)
    {
        case GOSSIP_ACTION_INFO_DEF:
            player->ADD_GOSSIP_ITEM( GOSSIP_ICON_CHAT, "Continuez s'il vous plait", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
            player->SEND_GOSSIP_MENU_TEXTID(1954, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+1:
            player->ADD_GOSSIP_ITEM( GOSSIP_ICON_CHAT, "Laissez-moi en discuter avec mes collègues", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
            player->SEND_GOSSIP_MENU_TEXTID(1955, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+2:
            player->CLOSE_GOSSIP_MENU();
            player->AreaExploredOrEventHappens(3441);
            break;
    }
    return true;
}

/*######
## npc_lothos_riftwaker
######*/

bool GossipHello_npc_lothos_riftwaker(Player *player, Creature *_Creature)
{
    if (_Creature->IsQuestGiver())
        player->PrepareQuestMenu( _Creature->GetGUID() );

    if (player->GetQuestRewardStatus(7487) || player->GetQuestRewardStatus(7848))
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Teleport me to Molten Core", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

    player->SEND_GOSSIP_MENU_TEXTID(player->GetGossipTextId(_Creature), _Creature->GetGUID());

    return true;
}

bool GossipSelect_npc_lothos_riftwaker(Player *player, Creature *_Creature, uint32 sender, uint32 action )
{
    if (action == GOSSIP_ACTION_INFO_DEF + 1)
    {
        player->CLOSE_GOSSIP_MENU();
        player->TeleportTo(409, 1096, -467, -104.6, 3.64);
    }

    return true;
}

/*######
## npc_zamael_lunthistle
######*/

bool GossipHello_npc_zamael_lunthistle(Player *player, Creature *_Creature)
{
    if (_Creature->IsQuestGiver())
        player->PrepareQuestMenu( _Creature->GetGUID() );

    if (player->GetQuestStatus(3377) == QUEST_STATUS_INCOMPLETE)
        player->ADD_GOSSIP_ITEM( GOSSIP_ICON_CHAT, "Racontez-moi votre histoire", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);

    player->SEND_GOSSIP_MENU_TEXTID(1920, _Creature->GetGUID());

    return true;
}

bool GossipSelect_npc_zamael_lunthistle(Player *player, Creature *_Creature, uint32 sender, uint32 action)
{
    switch (action)
    {
        case GOSSIP_ACTION_INFO_DEF:
            player->ADD_GOSSIP_ITEM( GOSSIP_ICON_CHAT, "Continuez...", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
            player->SEND_GOSSIP_MENU_TEXTID(1921, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+1:
            player->ADD_GOSSIP_ITEM( GOSSIP_ICON_CHAT, "Au revoir", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
            player->SEND_GOSSIP_MENU_TEXTID(1922, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+2:
            player->CLOSE_GOSSIP_MENU();
            player->AreaExploredOrEventHappens(3377);
            break;
    }
    return true;
}

/*######
##
######*/

void AddSC_searing_gorge()
{
    OLDScript *newscript;

    newscript = new OLDScript;
    newscript->Name="npc_kalaran_windblade";
    newscript->OnGossipHello =  &GossipHello_npc_kalaran_windblade;
    newscript->OnGossipSelect = &GossipSelect_npc_kalaran_windblade;
    sScriptMgr->RegisterOLDScript(newscript);

    newscript = new OLDScript;
    newscript->Name="npc_lothos_riftwaker";
    newscript->OnGossipHello          = &GossipHello_npc_lothos_riftwaker;
    newscript->OnGossipSelect         = &GossipSelect_npc_lothos_riftwaker;
    sScriptMgr->RegisterOLDScript(newscript);

    newscript = new OLDScript;
    newscript->Name="npc_zamael_lunthistle";
    newscript->OnGossipHello =  &GossipHello_npc_zamael_lunthistle;
    newscript->OnGossipSelect = &GossipSelect_npc_zamael_lunthistle;
    sScriptMgr->RegisterOLDScript(newscript);
}

