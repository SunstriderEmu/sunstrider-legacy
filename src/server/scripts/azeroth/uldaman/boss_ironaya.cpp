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
SDName: Boss_Ironaya
SD%Complete: 100
SDComment:
SDCategory: Uldaman
EndScriptData */



#define SPELL_ARCINGSMASH           39144
#define SPELL_KNOCKAWAY             22893
#define SPELL_WSTOMP                16727

#define SAY_AGGRO           "None may steal the secrets of the makers!"
#define SOUND_AGGRO         5851

struct boss_ironayaAI : public ScriptedAI
{
    boss_ironayaAI(Creature *c) : ScriptedAI(c) {}

    uint32 Arcing_Timer;
    bool hasCastedWstomp;
    bool hasCastedKnockaway;

    void Reset()
    override {
        Arcing_Timer = 3000;
        hasCastedKnockaway = false;
        hasCastedWstomp = false;
    }

    void EnterCombat(Unit *who)
    override {
        me->Yell(SAY_AGGRO,LANG_UNIVERSAL,nullptr);
        DoPlaySoundToSet(me,SOUND_AGGRO);
    }

    void UpdateAI(const uint32 diff)
    override {
        //Return since we have no target
        if (!UpdateVictim())
            return;

        //If we are <50% hp do knockaway ONCE
        if (!hasCastedKnockaway && me->GetHealth()*2 < me->GetMaxHealth())
        {
            me->CastSpell(me->GetVictim(),SPELL_KNOCKAWAY, true);

            // current aggro target is knocked away pick new target
            Unit* Target = SelectTarget(SELECT_TARGET_TOPAGGRO, 0);

            if (!Target || Target == me->GetVictim())
                Target = SelectTarget(SELECT_TARGET_TOPAGGRO, 1);

            if (Target)
                me->TauntApply(Target);

            //Shouldn't cast this agian
            hasCastedKnockaway = true;
        }

        //Arcing_Timer
        if (Arcing_Timer < diff)
        {
            DoCast(me,SPELL_ARCINGSMASH);
            Arcing_Timer = 13000;
        }else Arcing_Timer -= diff;

        if (!hasCastedWstomp && me->GetHealth()*4 < me->GetMaxHealth())
        {
            DoCast(me,SPELL_WSTOMP);
            hasCastedWstomp = true;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_ironaya(Creature *_Creature)
{
    return new boss_ironayaAI (_Creature);
}

void AddSC_boss_ironaya()
{
    OLDScript *newscript;
    newscript = new OLDScript;
    newscript->Name="boss_ironaya";
    newscript->GetAI = &GetAI_boss_ironaya;
    sScriptMgr->RegisterOLDScript(newscript);
}

