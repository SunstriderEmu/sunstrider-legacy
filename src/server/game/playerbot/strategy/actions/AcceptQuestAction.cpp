////
#include "../../playerbot.h"
#include "AcceptQuestAction.h"

using namespace ai;

void AcceptAllQuestsAction::ProcessQuest(Quest const* quest, WorldObject* questGiver)
{
    AcceptQuest(quest, questGiver->GetGUID());
}

bool AcceptQuestAction::Execute(Event event)
{
    Player* master = GetMaster();

    if (!master)
        return false;

    //Player* _bot = ai->GetBot();
    uint64 guid;
    uint32 quest;

    std::string text = event.getParam();
    PlayerbotChatHandler ch(master);
    quest = ch.extractQuestId(text);
    if (quest)
    {
        Unit* npc = master->GetSelectedUnit();
        if (!npc)
        {
            ai->TellMaster("Please select quest giver NPC");
            return false;
        }
        guid = npc->GetGUID();
    }
    else if (!event.getPacket().empty())
    {
        WorldPacket& p = event.getPacket();
        p.rpos(0);
        p >> guid >> quest;
    }
    else if (text == "*")
    {
        return QuestAction::Execute(event);
    }
    else
        return false;

    Quest const* qInfo = sObjectMgr->GetQuestTemplate(quest);
    if (!qInfo)
        return false;

    return AcceptQuest(qInfo, guid);
}

bool AcceptQuestShareAction::Execute(Event event)
{
    Player* master = GetMaster();
    Player* _bot = ai->GetBot();

    WorldPacket& p = event.getPacket();
    p.rpos(0);
    uint32 quest;
    p >> quest;
    Quest const* qInfo = sObjectMgr->GetQuestTemplate(quest);

    if (!qInfo || !_bot->GetDivider())
        return false;

    quest = qInfo->GetQuestId();
    if( !_bot->CanTakeQuest( qInfo, false ) )
    {
        // can't take quest
        _bot->SetDivider( ObjectGuid() );
        ai->TellMaster("I can't take this quest");

        return false;
    }

    // send msg to quest giving player
    master->SendPushToPartyResponse(_bot, QUEST_PARTY_MSG_ACCEPT_QUEST );
    _bot->SetDivider( ObjectGuid() );

    if(_bot->CanAddQuest( qInfo, false ) )
    {
        _bot->AddQuest( qInfo, master );

        if(_bot->CanCompleteQuest( quest ) )
            _bot->CompleteQuest( quest );

        // Runsttren: did not add typeid switch from WorldSession::HandleQuestgiverAcceptQuestOpcode!
        // I think it's not needed, cause typeid should be TYPEID_PLAYER - and this one is not handled
        // there and there is no default case also.

        if( qInfo->GetSrcSpell() > 0 )
            _bot->CastSpell(_bot, qInfo->GetSrcSpell(), TRIGGERED_FULL_MASK);

        ai->TellMaster("Quest accepted");
        return true;
    }

    return false;
}
