#include "Chat.h"
#include "Language.h"

static bool HandleCastCommand(ChatHandler* handler, char const* args)
{
    ARGS_CHECK

    Unit* target = GetSelectedUnit();

    if(!target)
    {
        handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        handler->SetSentErrorMessage(true);
        return false;
    }

    // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
    uint32 spell = extractSpellIdFromLink((char*)args);
    if(!spell)
        return false;

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spell);
    if(!spellInfo)
        return false;

    if(!SpellMgr::IsSpellValid(spellInfo,m_session->GetPlayer()))
    {
        handler->PSendSysMessage(LANG_COMMAND_SPELL_BROKEN,spell);
        handler->SetSentErrorMessage(true);
        return false;
    }

    char* trig_str = strtok(nullptr, " ");
    if(trig_str)
    {
        int l = strlen(trig_str);
        if(strncmp(trig_str,"triggered",l) != 0 )
            return false;
    }

    bool triggered = (trig_str != nullptr);

    m_session->GetPlayer()->CastSpell(target,spell, triggered ? TRIGGERED_FULL_MASK : TRIGGERED_NONE);

    return true;
}

/** args : <spellid> <count> */
static bool HandleCastBatchCommand(ChatHandler* handler, char const* args)
{
    Unit* target = GetSelectedUnit();

    if(!target)
    {
        handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        handler->SetSentErrorMessage(true);
        return false;
    }

    // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r
    // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
    uint32 spell = extractSpellIdFromLink((char*)args);
    if(!spell || !sSpellMgr->GetSpellInfo(spell))
        return false;

    char* countStr = strtok(nullptr, " ");
    int count = 0;
    if(countStr)
    {
        count = atoi(countStr);
        if(count == 0)
            return false;
    }

    for(int i = 0; i < count; i++)
        m_session->GetPlayer()->CastSpell(target, spell, true);

    return true;
}

static bool HandleCastBackCommand(ChatHandler* handler, char const* args)
{
    Unit* caster = GetSelectedUnit();

    if(!caster)
    {
        handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        handler->SetSentErrorMessage(true);
        return false;
    }

    // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r
    // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
    uint32 spell = extractSpellIdFromLink((char*)args);
    if(!spell || !sSpellMgr->GetSpellInfo(spell))
        return false;

    char* trig_str = strtok(nullptr, " ");
    if(trig_str)
    {
        int l = strlen(trig_str);
        if(strncmp(trig_str,"triggered",l) != 0 )
            return false;
    }

    bool triggered = (trig_str != nullptr);

    // update orientation at server
    caster->SetOrientation(caster->GetAbsoluteAngle(m_session->GetPlayer()));

    // and client
    WorldPacket data;
    caster->BuildHeartBeatMsg(&data);
    caster->SendMessageToSet(&data,true);

    caster->CastSpell(m_session->GetPlayer(),spell, triggered ? TRIGGERED_FULL_MASK : TRIGGERED_NONE);

    return true;
}

static bool HandleCastDistCommand(ChatHandler* handler, char const* args)
{
    ARGS_CHECK

    // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
    uint32 spell = extractSpellIdFromLink((char*)args);
    if(!spell)
        return false;

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spell);
    if(!spellInfo)
        return false;

    if(!SpellMgr::IsSpellValid(spellInfo,m_session->GetPlayer()))
    {
        handler->PSendSysMessage(LANG_COMMAND_SPELL_BROKEN,spell);
        handler->SetSentErrorMessage(true);
        return false;
    }

    char *distStr = strtok(nullptr, " ");

    float dist = 0;

    if(distStr)
        sscanf(distStr, "%f", &dist);

    char* trig_str = strtok(nullptr, " ");
    if(trig_str)
    {
        int l = strlen(trig_str);
        if(strncmp(trig_str,"triggered",l) != 0 )
            return false;
    }

    TriggerCastFlags triggered = (trig_str != nullptr) ? TRIGGERED_FULL_DEBUG_MASK : TRIGGERED_NONE;

    float x,y,z;
    m_session->GetPlayer()->GetClosePoint(x,y,z,dist);
    m_session->GetPlayer()->CastSpell({ x,y,z }, spell, triggered);
    return true;
}

static bool HandleCastTargetCommand(ChatHandler* handler, char const* args)
{
    Creature* caster = GetSelectedCreature();

    if(!caster)
    {
        handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        handler->SetSentErrorMessage(true);
        return false;
    }

    if(!caster->GetVictim())
    {
        handler->SendSysMessage(LANG_SELECTED_TARGET_NOT_HAVE_VICTIM);
        handler->SetSentErrorMessage(true);
        return false;
    }

    // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
    uint32 spell = extractSpellIdFromLink((char*)args);
    if(!spell || !sSpellMgr->GetSpellInfo(spell))
        return false;

    char* trig_str = strtok(nullptr, " ");
    if(trig_str)
    {
        int l = strlen(trig_str);
        if(strncmp(trig_str,"triggered",l) != 0 )
            return false;
    }

    bool triggered = (trig_str != nullptr);

    // update orientation at server
    caster->SetOrientation(caster->GetAbsoluteAngle(m_session->GetPlayer()));

    // and client
    WorldPacket data;
    caster->BuildHeartBeatMsg(&data);
    caster->SendMessageToSet(&data,true);

    caster->CastSpell(caster->GetVictim(),spell, triggered ? TRIGGERED_FULL_MASK : TRIGGERED_NONE);

    return true;
}

static bool HandleCastSelfCommand(ChatHandler* handler, char const* args)
{
    ARGS_CHECK

    Unit* target = GetSelectedUnit();

    if(!target)
    {
        handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        handler->SetSentErrorMessage(true);
        return false;
    }

    // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
    uint32 spell = extractSpellIdFromLink((char*)args);
    if(!spell)
        return false;

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spell);
    if(!spellInfo)
        return false;

    if(!SpellMgr::IsSpellValid(spellInfo,m_session->GetPlayer()))
    {
        handler->PSendSysMessage(LANG_COMMAND_SPELL_BROKEN,spell);
        handler->SetSentErrorMessage(true);
        return false;
    }

    target->CastSpell(target,spell, TRIGGERED_NONE);

    return true;
}