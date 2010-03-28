/* ScriptData
SDName: Durotar
SD%Complete: 100
SDComment: Quest support: 5441
SDCategory: Durotar
EndScriptData */

/* ContentData
npc_lazy_peon
EndContentData */

#include "precompiled.h"

/*######
## npc_lazy_peon
######*/

enum LazyPeonYells
{
    SAY_SPELL_HIT                               = -1645010   //Ow! OK, I''ll get back to work, $N!'
};

enum LazyPeon
{
    GO_LUMBERPILE                               = 175784,

    SPELL_BUFF_SLEEP                            = 17743,
    SPELL_AWAKEN_PEON                           = 19938
};

struct npc_lazy_peonAI : public ScriptedAI
{
    npc_lazy_peonAI(Creature *c) : ScriptedAI(c) {}

    uint32 m_uiRebuffTimer;
    uint32 work;

    void Reset ()
    {
        work = false;
    }

    void MovementInform(uint32, uint32 id)
    {
        if (id == 1)
            work = true;
    }
    
    void Aggro(Unit* pWho) {}

    void SpellHit(Unit *caster, const SpellEntry *spell)
    {
        if (spell->Id == SPELL_AWAKEN_PEON && caster->GetTypeId() == TYPEID_PLAYER
            && CAST_PLR(caster)->GetQuestStatus(5441) == QUEST_STATUS_INCOMPLETE)
        {
            ((Player *)caster)->KilledMonster(m_creature->GetEntry(), m_creature->GetGUID());
            DoScriptText(SAY_SPELL_HIT, m_creature, caster);
            m_creature->RemoveAllAuras();
            GameObject* Lumberpile = m_creature->FindGOInGrid(GO_LUMBERPILE, 20);
            if(Lumberpile)
                m_creature->GetMotionMaster()->MovePoint(1,Lumberpile->GetPositionX()-1,Lumberpile->GetPositionY(),Lumberpile->GetPositionZ());
        }
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (work = true)
            m_creature->HandleEmoteCommand(466);
        if (m_uiRebuffTimer <= uiDiff)
        {
            DoCast(m_creature, SPELL_BUFF_SLEEP);
            m_uiRebuffTimer = 300000;                 // Rebuff again in 5 minutes
        }
        else
            m_uiRebuffTimer -= uiDiff;
        if (!UpdateVictim())
            return;
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_lazy_peon(Creature* pCreature)
{
    return new npc_lazy_peonAI(pCreature);
}

void AddSC_durotar()
{
    Script* newscript;

    newscript = new Script;
    newscript->Name = "npc_lazy_peon";
    newscript->GetAI = &GetAI_npc_lazy_peon;
    newscript->RegisterSelf();
}
