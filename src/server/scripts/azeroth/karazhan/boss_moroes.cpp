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
SDName: Boss_Moroes
SD%Complete: 95
SDComment:
SDCategory: Karazhan
EndScriptData */


#include "def_karazhan.h"

#define SAY_AGGRO           -1532011
#define SAY_SPECIAL_1       -1532012
#define SAY_SPECIAL_2       -1532013
#define SAY_KILL_1          -1532014
#define SAY_KILL_2          -1532015
#define SAY_KILL_3          -1532016
#define SAY_DEATH           -1532017

#define SPELL_VANISH        29448
#define SPELL_GARROTE       37066
#define SPELL_BLIND         34694
#define SPELL_GOUGE         29425
#define SPELL_FRENZY        37023

#define POS_Z               81.73

float Locations[4][3]=
{
    {-10991.0, -1884.33, 0.614315},
    {-10989.4, -1885.88, 0.904913},
    {-10978.1, -1887.07, 2.035550},
    {-10975.9, -1885.81, 2.253890},
};

const uint32 Adds[6]=
{
    17007,
    19872,
    19873,
    19874,
    19875,
    19876,
};

struct boss_moroesAI : public ScriptedAI
{
    boss_moroesAI(Creature *c) : ScriptedAI(c)
    {
        for(uint32 & i : AddId)
        {
            i = 0;
        }
        pInstance = ((InstanceScript*)c->GetInstanceScript());
    }

    InstanceScript *pInstance;

    uint64 AddGUID[4];

    uint32 Vanish_Timer;
    uint32 Blind_Timer;
    uint32 Gouge_Timer;
    uint32 Wait_Timer;
    uint32 CheckAdds_Timer;
    uint32 AddId[4];

    bool InVanish;
    bool Enrage;

    void Reset()
    override {
        Vanish_Timer = 30000;
        Blind_Timer = 35000;
        Gouge_Timer = 23000;
        Wait_Timer = 0;
        CheckAdds_Timer = 5000;

        Enrage = false;
        InVanish = false;
        if(me->GetHealth() > 0)
        {
            SpawnAdds();
        }

        if(pInstance)
            pInstance->SetData(DATA_MOROES_EVENT, NOT_STARTED);
    }

    void StartEvent()
    {
        if(pInstance)
            pInstance->SetData(DATA_MOROES_EVENT, IN_PROGRESS);

        DoZoneInCombat();
    }

    void EnterCombat(Unit* who)
    override {
        StartEvent();

        DoScriptText(SAY_AGGRO, me);
        AddsAttack();
        DoZoneInCombat();
    }

    void KilledUnit(Unit* victim)
    override {
        switch (rand()%3)
        {
        case 0: DoScriptText(SAY_KILL_1, me); break;
        case 1: DoScriptText(SAY_KILL_2, me); break;
        case 2: DoScriptText(SAY_KILL_3, me); break;
        }
    }

    void JustDied(Unit* victim)
    override {
        DoScriptText(SAY_DEATH, me);

        if (pInstance)
            pInstance->SetData(DATA_MOROES_EVENT, DONE);

        DeSpawnAdds();

        //remove aura from spell Garrote when Moroes dies
        Map *map = me->GetMap();
        if (map->IsDungeon())
        {
            Map::PlayerList const &PlayerList = map->GetPlayers();

            if (PlayerList.isEmpty())
                return;

            for (const auto & i : PlayerList)
            {
                if (i.GetSource()->IsAlive() && i.GetSource()->HasAuraEffect(SPELL_GARROTE,0))
                    i.GetSource()->RemoveAurasDueToSpell(SPELL_GARROTE);
            }
        }
    }

    void SpawnAdds()
    {
        DeSpawnAdds();
        if(isAddlistEmpty())
        {
            Creature *pCreature = nullptr;
            std::vector<uint32> AddList;


            for(uint32 Add : Adds)
                AddList.push_back(Add);

            while(AddList.size() > 4)
                AddList.erase((AddList.begin())+(rand()%AddList.size()));

            uint8 i = 0;
            for(uint32 entry : AddList)
            {
                pCreature = me->SummonCreature(entry, Locations[i][0], Locations[i][1], POS_Z, Locations[i][2], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                if (pCreature)
                {
                    AddGUID[i] = pCreature->GetGUID();
                    AddId[i] = entry;
                }
                ++i;
            }
        }else
        {
            for(int i = 0; i < 4; i++)
            {
                Creature *pCreature = me->SummonCreature(AddId[i], Locations[i][0], Locations[i][1], POS_Z, Locations[i][2], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);                
                if (pCreature)
                {
                    AddGUID[i] = pCreature->GetGUID();
                }
            }
        }
    }

    bool isAddlistEmpty()
    {
        for(uint32 i : AddId)
        {
            if(i == 0)
                return true;
        }
        return false;
    }

    void DeSpawnAdds()
    {
        for(uint64 i : AddGUID)
        {
            Creature* Temp = nullptr;
            if (i)
            {
                Temp = ObjectAccessor::GetCreature((*me),i);
                if (Temp && Temp->IsAlive())
                {
                    (*Temp).GetMotionMaster()->Clear(true);
                    Temp->DealDamage(Temp, Temp->GetMaxHealth(), nullptr, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, nullptr, false);
                    Temp->RemoveCorpse();
                }

            }
        }
    }

    void AddsAttack()
    {
        for(uint64 i : AddGUID)
        {
            Creature* Temp = nullptr;
            if (i)
            {
                Temp = ObjectAccessor::GetCreature((*me),i);
                if (Temp && Temp->IsAlive())
                {
                    Temp->AI()->AttackStart(me->GetVictim());
                    DoZoneInCombat(Temp);
                }else
                    EnterEvadeMode();
            }
        }
    }

    void UpdateAI(const uint32 diff)
    override {
        if(!UpdateVictim() )
            return;

        if(pInstance && !pInstance->GetData(DATA_MOROES_EVENT))
        {
            EnterEvadeMode();
            return;
        }

        if(!Enrage && me->GetHealth()*100 / me->GetMaxHealth() < 30)
        {
            DoCast(me, SPELL_FRENZY);
            Enrage = true;
        }

        if (CheckAdds_Timer < diff)
        {
            for (uint64 i : AddGUID)
            {
                Creature* Temp = nullptr;
                if (i)
                {
                    Temp = ObjectAccessor::GetCreature((*me),i);
                    if (Temp && Temp->IsAlive())
                        if (!Temp->GetVictim() )
                            Temp->AI()->AttackStart(me->GetVictim());
                }
            }
            CheckAdds_Timer = 5000;
        }else CheckAdds_Timer -= diff;

        if (!Enrage)
        {
            //Cast Vanish, then Garrote random victim
            if (Vanish_Timer < diff)
            {
                DoCast(me, SPELL_VANISH);
                InVanish = true;
                Vanish_Timer = 30000;
                Wait_Timer = 5000;
            }else Vanish_Timer -= diff;

            if (Gouge_Timer < diff)
            {
                DoCast(me->GetVictim(), SPELL_GOUGE);
                Gouge_Timer = 40000;
            }else Gouge_Timer -= diff;

            if (Blind_Timer < diff)
            {
                Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 5, true);
                if (target && me->IsWithinMeleeRange(target))
                    DoCast(target, SPELL_BLIND);

                Blind_Timer = 40000;
            }else Blind_Timer -= diff;
        }

        if (InVanish)
        {
            if (Wait_Timer < diff)
            {
                switch(rand()%2)
                {
                    case 0: DoScriptText(SAY_SPECIAL_1, me); break;
                    case 1: DoScriptText(SAY_SPECIAL_2, me); break;
                }

                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 80.0f, true))
                   target->CastSpell(target, SPELL_GARROTE,true);

                InVanish = false;
            }else Wait_Timer -= diff;
        }

        if (!InVanish)
            DoMeleeAttackIfReady();
    }
};

struct boss_moroes_guestAI : public ScriptedAI
{
    InstanceScript* pInstance;

    uint64 GuestGUID[4];

    boss_moroes_guestAI(Creature* c) : ScriptedAI(c)
    {
        for(uint64 & i : GuestGUID)
            i = 0;

        pInstance = ((InstanceScript*)c->GetInstanceScript());
    }

    void Reset()
    override {
        if(pInstance)
            pInstance->SetData(DATA_MOROES_EVENT, NOT_STARTED);
    }

    void EnterCombat(Unit* who) override {}

    void AcquireGUID()
    {
        if(!pInstance)
            return;

        GuestGUID[0] = pInstance->GetData64(DATA_MOROES);
        Creature* Moroes = (ObjectAccessor::GetCreature((*me), GuestGUID[0]));
        if(Moroes)
        {
            for(uint8 i = 0; i < 3; ++i)
            {
                uint64 GUID = ((boss_moroesAI*)Moroes->AI())->AddGUID[i];
                if(GUID && GUID != me->GetGUID())
                    GuestGUID[i+1] = GUID;
            }
        }
    }

    Unit* SelectRandomGuest()
    {
        uint64 TempGUID = GuestGUID[rand()%5];
        if(TempGUID)
        {
            Unit* pUnit = ObjectAccessor::GetUnit((*me), TempGUID);
            if(pUnit && pUnit->IsAlive())
                return pUnit;
        }

        return me;
    }

    void UpdateAI(const uint32 diff)
    override {
        if(pInstance && !pInstance->GetData(DATA_MOROES_EVENT))
            EnterEvadeMode();

        DoMeleeAttackIfReady();
    }
};

#define SPELL_MANABURN       29405
#define SPELL_MINDFLY        29570
#define SPELL_SHADOWFORM     29406

struct boss_baroness_dorothea_millstipeAI : public boss_moroes_guestAI
{
    //Shadow Priest
    boss_baroness_dorothea_millstipeAI(Creature *c) : boss_moroes_guestAI(c) {}

    uint32 ManaBurn_Timer;
    uint32 MindFlay_Timer;

    void Reset()
    override {
        ManaBurn_Timer = 7000;
        MindFlay_Timer = 1000;

        DoCast(me,SPELL_SHADOWFORM, true);

        boss_moroes_guestAI::Reset();
    }

    void UpdateAI(const uint32 diff)
    override {
        if(!UpdateVictim() )
            return;

        boss_moroes_guestAI::UpdateAI(diff);

        if(MindFlay_Timer < diff)
        {
            DoCast(me->GetVictim(),SPELL_MINDFLY);
            MindFlay_Timer = 12000;                         //3sec channeled
        }else MindFlay_Timer -= diff;

        if(ManaBurn_Timer < diff)
        {
            Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0);
            if(target && (target->GetPowerType() == POWER_MANA))
                DoCast(target,SPELL_MANABURN);
            ManaBurn_Timer = 5000;                          //3 sec cast
        }else ManaBurn_Timer -= diff;
    }
};

#define SPELL_HAMMEROFJUSTICE       13005
#define SPELL_JUDGEMENTOFCOMMAND    29386
#define SPELL_SEALOFCOMMAND         29385

struct boss_baron_rafe_dreugerAI : public boss_moroes_guestAI
{
    //Retr Pally
    boss_baron_rafe_dreugerAI(Creature *c) : boss_moroes_guestAI(c){}

    uint32 HammerOfJustice_Timer;
    uint32 SealOfCommand_Timer;
    uint32 JudgementOfCommand_Timer;

    void Reset()
    override {
        HammerOfJustice_Timer = 1000;
        SealOfCommand_Timer = 7000;
        JudgementOfCommand_Timer = SealOfCommand_Timer + 29000;

        boss_moroes_guestAI::Reset();
    }

    void UpdateAI(const uint32 diff)
    override {
        if(!UpdateVictim() )
            return;

        boss_moroes_guestAI::UpdateAI(diff);

        if(SealOfCommand_Timer < diff)
        {
            DoCast(me,SPELL_SEALOFCOMMAND);
            SealOfCommand_Timer = 32000;
            JudgementOfCommand_Timer = 29000;
        }else SealOfCommand_Timer -= diff;

        if(JudgementOfCommand_Timer < diff)
        {
            DoCast(me->GetVictim(),SPELL_JUDGEMENTOFCOMMAND);
            JudgementOfCommand_Timer = SealOfCommand_Timer + 29000;
        }else JudgementOfCommand_Timer -= diff;

        if(HammerOfJustice_Timer < diff)
        {
            DoCast(me->GetVictim(),SPELL_HAMMEROFJUSTICE);
            HammerOfJustice_Timer = 12000;
        }else HammerOfJustice_Timer -= diff;
    }
};

#define SPELL_DISPELMAGIC           15090                   //Self or other guest+Moroes
#define SPELL_GREATERHEAL           29564                   //Self or other guest+Moroes
#define SPELL_HOLYFIRE              29563
#define SPELL_PWSHIELD              29408

struct boss_lady_catriona_von_indiAI : public boss_moroes_guestAI
{
    //Holy Priest
    boss_lady_catriona_von_indiAI(Creature *c) : boss_moroes_guestAI(c) {}

    uint32 DispelMagic_Timer;
    uint32 GreaterHeal_Timer;
    uint32 HolyFire_Timer;
    uint32 PowerWordShield_Timer;

    void Reset()
    override {
        DispelMagic_Timer = 11000;
        GreaterHeal_Timer = 1500;
        HolyFire_Timer = 5000;
        PowerWordShield_Timer = 1000;

        AcquireGUID();

        boss_moroes_guestAI::Reset();
    }

    void UpdateAI(const uint32 diff)
    override {
        if(!UpdateVictim() )
            return;

        boss_moroes_guestAI::UpdateAI(diff);

        if(PowerWordShield_Timer < diff)
        {
            DoCast(me,SPELL_PWSHIELD);
            PowerWordShield_Timer = 15000;
        }else PowerWordShield_Timer -= diff;

        if(GreaterHeal_Timer < diff)
        {
            Unit* target = SelectRandomGuest();

            DoCast(target, SPELL_GREATERHEAL);
            GreaterHeal_Timer = 17000;
        }else GreaterHeal_Timer -= diff;

        if(HolyFire_Timer < diff)
        {
            DoCast(me->GetVictim(),SPELL_HOLYFIRE);
            HolyFire_Timer = 22000;
        }else HolyFire_Timer -= diff;

        if(DispelMagic_Timer < diff)
        {
            if(rand()%2)
            {
                Unit* target = SelectRandomGuest();

                DoCast(target, SPELL_DISPELMAGIC);
            }
            else
                DoCast(SelectTarget(SELECT_TARGET_RANDOM, 0), SPELL_DISPELMAGIC);

            DispelMagic_Timer = 25000;
        }else DispelMagic_Timer -= diff;
    }
};

#define SPELL_CLEANSE               29380                   //Self or other guest+Moroes
#define SPELL_GREATERBLESSOFMIGHT   29381                   //Self or other guest+Moroes
#define SPELL_HOLYLIGHT             29562                   //Self or other guest+Moroes
#define SPELL_DIVINESHIELD          29382

struct boss_lady_keira_berrybuckAI : public boss_moroes_guestAI
{
    //Holy Pally
    boss_lady_keira_berrybuckAI(Creature *c) : boss_moroes_guestAI(c)  {}

    uint32 Cleanse_Timer;
    uint32 GreaterBless_Timer;
    uint32 HolyLight_Timer;
    uint32 DivineShield_Timer;

    void Reset()
    override {
        Cleanse_Timer = 13000;
        GreaterBless_Timer = 1000;
        HolyLight_Timer = 7000;
        DivineShield_Timer = 31000;

        AcquireGUID();

        boss_moroes_guestAI::Reset();
    }

    void UpdateAI(const uint32 diff)
    override {
        if(!UpdateVictim() )
            return;

        boss_moroes_guestAI::UpdateAI(diff);

        if(DivineShield_Timer < diff)
        {
            DoCast(me,SPELL_DIVINESHIELD);
            DivineShield_Timer = 31000;
        }else DivineShield_Timer -= diff;

        if(HolyLight_Timer < diff)
        {
            Unit* target = SelectRandomGuest();

            DoCast(target, SPELL_HOLYLIGHT);
            HolyLight_Timer = 10000;
        }else HolyLight_Timer -= diff;

        if(GreaterBless_Timer < diff)
        {
            Unit* target = SelectRandomGuest();

            DoCast(target, SPELL_GREATERBLESSOFMIGHT);

            GreaterBless_Timer = 50000;
        }else GreaterBless_Timer -= diff;

        if(Cleanse_Timer < diff)
        {
            Unit* target = SelectRandomGuest();

            DoCast(target, SPELL_CLEANSE);

            Cleanse_Timer = 10000;
        }else Cleanse_Timer -= diff;
    }
};

#define SPELL_HAMSTRING         9080
#define SPELL_MORTALSTRIKE      29572
#define SPELL_WHIRLWIND         29573

struct boss_lord_robin_darisAI : public boss_moroes_guestAI
{
    //Arms Warr
    boss_lord_robin_darisAI(Creature *c) : boss_moroes_guestAI(c) {}

    uint32 Hamstring_Timer;
    uint32 MortalStrike_Timer;
    uint32 WhirlWind_Timer;

    void Reset()
    override {
        Hamstring_Timer = 7000;
        MortalStrike_Timer = 10000;
        WhirlWind_Timer = 21000;

        boss_moroes_guestAI::Reset();
    }

    void UpdateAI(const uint32 diff)
    override {
        if(!UpdateVictim() )
            return;

        boss_moroes_guestAI::UpdateAI(diff);

        if(Hamstring_Timer < diff)
        {
            DoCast(me->GetVictim(),SPELL_HAMSTRING);
            Hamstring_Timer = 12000;
        }else Hamstring_Timer -= diff;

        if(MortalStrike_Timer < diff)
        {
            DoCast(me->GetVictim(), SPELL_MORTALSTRIKE);
            MortalStrike_Timer = 18000;
        }else MortalStrike_Timer -= diff;

        if(WhirlWind_Timer < diff)
        {
            DoCast(me,SPELL_WHIRLWIND);
            WhirlWind_Timer = 21000;
        }else WhirlWind_Timer -= diff;
    }
};

#define SPELL_DISARM            8379
#define SPELL_HEROICSTRIKE      29567
#define SPELL_SHIELDBASH        11972
#define SPELL_SHIELDWALL        29390

struct boss_lord_crispin_ferenceAI : public boss_moroes_guestAI
{
    //Arms Warr
    boss_lord_crispin_ferenceAI(Creature *c) : boss_moroes_guestAI(c) {}

    uint32 Disarm_Timer;
    uint32 HeroicStrike_Timer;
    uint32 ShieldBash_Timer;
    uint32 ShieldWall_Timer;

    void Reset()
    override {
        Disarm_Timer = 6000;
        HeroicStrike_Timer = 10000;
        ShieldBash_Timer = 8000;
        ShieldWall_Timer = 4000;

        boss_moroes_guestAI::Reset();
    }

    void UpdateAI(const uint32 diff)
    override {
        if(!UpdateVictim() )
            return;

        boss_moroes_guestAI::UpdateAI(diff);

        if(Disarm_Timer < diff)
        {
            DoCast(me->GetVictim(),SPELL_DISARM);
            Disarm_Timer = 12000;
        }else Disarm_Timer -= diff;

        if(HeroicStrike_Timer < diff)
        {
            DoCast(me->GetVictim(),SPELL_HEROICSTRIKE);
            HeroicStrike_Timer = 10000;
        }else HeroicStrike_Timer -= diff;

        if(ShieldBash_Timer < diff)
        {
            DoCast(me->GetVictim(),SPELL_SHIELDBASH);
            ShieldBash_Timer = 13000;
        }else ShieldBash_Timer -= diff;

        if(ShieldWall_Timer < diff)
        {
            DoCast(me,SPELL_SHIELDWALL);
            ShieldWall_Timer = 21000;
        }else ShieldWall_Timer -= diff;
    }
};

CreatureAI* GetAI_boss_moroes(Creature *_Creature)
{
    return new boss_moroesAI (_Creature);
}

CreatureAI* GetAI_baroness_dorothea_millstipe(Creature *_Creature)
{
    return new boss_baroness_dorothea_millstipeAI (_Creature);
}

CreatureAI* GetAI_baron_rafe_dreuger(Creature *_Creature)
{
    return new boss_baron_rafe_dreugerAI (_Creature);
}

CreatureAI* GetAI_lady_catriona_von_indi(Creature *_Creature)
{
    return new boss_lady_catriona_von_indiAI (_Creature);
}

CreatureAI* GetAI_lady_keira_berrybuck(Creature *_Creature)
{
    return new boss_lady_keira_berrybuckAI (_Creature);
}

CreatureAI* GetAI_lord_robin_daris(Creature *_Creature)
{
    return new boss_lord_robin_darisAI (_Creature);
}

CreatureAI* GetAI_lord_crispin_ference(Creature *_Creature)
{
    return new boss_lord_crispin_ferenceAI (_Creature);
}

void AddSC_boss_moroes()
{
    OLDScript *newscript;

    newscript = new OLDScript;
    newscript->Name="boss_moroes";
    newscript->GetAI = &GetAI_boss_moroes;
    sScriptMgr->RegisterOLDScript(newscript);

    newscript = new OLDScript;
    newscript->Name="boss_baroness_dorothea_millstipe";
    newscript->GetAI = &GetAI_baroness_dorothea_millstipe;
    sScriptMgr->RegisterOLDScript(newscript);

    newscript = new OLDScript;
    newscript->Name="boss_baron_rafe_dreuger";
    newscript->GetAI = &GetAI_baron_rafe_dreuger;
    sScriptMgr->RegisterOLDScript(newscript);

    newscript = new OLDScript;
    newscript->Name="boss_lady_catriona_von_indi";
    newscript->GetAI = &GetAI_lady_catriona_von_indi;
    sScriptMgr->RegisterOLDScript(newscript);

    newscript = new OLDScript;
    newscript->Name="boss_lady_keira_berrybuck";
    newscript->GetAI = &GetAI_lady_keira_berrybuck;
    sScriptMgr->RegisterOLDScript(newscript);

    newscript = new OLDScript;
    newscript->Name="boss_lord_robin_daris";
    newscript->GetAI = &GetAI_lord_robin_daris;
    sScriptMgr->RegisterOLDScript(newscript);

    newscript = new OLDScript;
    newscript->Name="boss_lord_crispin_ference";
    newscript->GetAI = &GetAI_lord_crispin_ference;
    sScriptMgr->RegisterOLDScript(newscript);
}

