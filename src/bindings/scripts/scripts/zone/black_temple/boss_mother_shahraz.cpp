/* Copyright (C) 2006 - 2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
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
SDName: Boss_Mother_Shahraz
SD%Complete: 80
SDComment: Saber Lash missing, Fatal Attraction slightly incorrect; need to damage only if affected players are within range of each other
SDCategory: Black Temple
EndScriptData */

#include "precompiled.h"
#include "def_black_temple.h"

//Speech'n'Sounds
enum Says
{
    SAY_TAUNT1              = -1564018,
    SAY_TAUNT2              = -1564019,
    SAY_TAUNT3              = -1564020,
    SAY_AGGRO               = -1564021,
    SAY_SPELL1              = -1564022,
    SAY_SPELL2              = -1564023,
    SAY_SPELL3              = -1564024,
    SAY_SLAY1               = -1564025,
    SAY_SLAY2               = -1564026,
    SAY_ENRAGE              = -1564027,
    SAY_DEATH               = -1564028
};

//Spells
enum Spells
{
    SPELL_BEAM_SINISTER             = 40859,
    SPELL_BEAM_SINISTER_TRIGGER     = 40863,
    SPELL_BEAM_VILE                 = 40860,
    SPELL_BEAM_VILE_TRIGGER         = 40865,
    SPELL_BEAM_WICKED               = 40861,
    SPELL_BEAM_WICKED_TRIGGER       = 40866,
    SPELL_BEAM_SINFUL               = 40827,
    SPELL_BEAM_SINFUL_TRIGGER       = 40862,
    SPELL_ATTRACTION                = 40871,
    SPELL_ATTRACTION_VIS            = 41001,
    SPELL_SILENCING_SHRIEK          = 40823,
    SPELL_ENRAGE                    = 23537,
    SPELL_SABER_LASH                = 40810,//43267
    SPELL_SABER_LASH_TRIGGER        = 40816,
    SPELL_SABER_LASH_IMM            = 43690,
    SPELL_TELEPORT_VISUAL           = 40869,
    SPELL_BERSERK                   = 45078,
    
    SPELL_PRISMATIC_SHIELD          = 40879
};

uint32 PrismaticAuras[]=
{
    40880,                                                  // Shadow
    40882,                                                  // Fire
    40883,                                                  // Nature
    40891,                                                  // Arcane
    40896,                                                  // Frost
    40897,                                                  // Holy
};

struct Locations
{
    float x,y,z;
};

static Locations TeleportPoint[]=
{
    /*{959.996, 212.576, 193.843},
    {932.537, 231.813, 193.838},
    {958.675, 254.767, 193.822},
    {946.955, 201.316, 192.535},
    {944.294, 149.676, 197.551},
    {930.548, 284.888, 193.367},
    {965.997, 278.398, 195.777}*/
    {959.996, 212.576, 195.215},
    {932.537, 231.813, 195.215},
    {958.675, 254.767, 195.215},
    {946.955, 201.316, 192.535},
    {944.294, 149.676, 200.175},
    {930.548, 284.888, 195.215},
    {965.997, 278.398, 198.215}
};

struct TRINITY_DLL_DECL boss_shahrazAI : public ScriptedAI
{
    boss_shahrazAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = ((ScriptedInstance*)c->GetInstanceData());
    }

    ScriptedInstance* pInstance;

    uint64 TargetGUID[3];
    uint32 BeamTimer;
    uint32 BeamCount;
    uint32 CurrentBeam;
    uint32 PrismaticShieldTimer;
    uint32 FatalAttractionTimer;
    uint32 FatalAttractionExplodeTimer;
    uint32 ShriekTimer;
    uint32 SaberTimer;
    uint32 RandomYellTimer;
    uint32 EnrageTimer;
    uint32 LastPrismaticAura;
    uint32 CheckPlayersUndermapTimer;
    //uint32 ExplosionCount;

    bool Enraged;

    void Reset()
    {
        if(pInstance && m_creature->isAlive())
            pInstance->SetData(DATA_MOTHERSHAHRAZEVENT, NOT_STARTED);

        for(uint8 i = 0; i<3; i++)
            TargetGUID[i] = 0;

        BeamTimer = 5000; // Timers may be incorrect
        CurrentBeam = rand()%4;                                    // 0 - Sinister, 1 - Vile, 2 - Wicked, 3 - Sinful
        PrismaticShieldTimer = 0;
        FatalAttractionTimer = 60000;
        FatalAttractionExplodeTimer = 70000;
        ShriekTimer = 30000;
        SaberTimer = 35000;
        RandomYellTimer = 70000 + rand()%41 * 1000;
        EnrageTimer = 600000;
        LastPrismaticAura = 0;
        CheckPlayersUndermapTimer = 5000;

        Enraged = false;
    }

    void Aggro(Unit *who)
    {
        if(pInstance)
            pInstance->SetData(DATA_MOTHERSHAHRAZEVENT, IN_PROGRESS);

        DoZoneInCombat();
        DoScriptText(SAY_AGGRO, m_creature);
        DoCast(m_creature,SPELL_PRISMATIC_SHIELD,true);
        //DoCast(m_creature,SPELL_SABER_LASH_TRIGGER,true);
    }

    void KilledUnit(Unit *victim)
    {
        DoScriptText(RAND(SAY_SLAY1,SAY_SLAY2), m_creature);
    }

    void JustDied(Unit *victim)
    {
        if(pInstance)
            pInstance->SetData(DATA_MOTHERSHAHRAZEVENT, DONE);

        DoScriptText(SAY_DEATH, m_creature);
    }

    void TeleportPlayers()
    {
        uint32 random = rand()%7;
        float X = TeleportPoint[random].x;
        float Y = TeleportPoint[random].y;
        float Z = TeleportPoint[random].z;
        for(uint8 i = 0; i < 3; i++)
        {
            Unit* pUnit = SelectUnit(SELECT_TARGET_RANDOM, 1);
            if(pUnit && pUnit->isAlive() && (pUnit->GetTypeId() == TYPEID_PLAYER) && !pUnit->HasAura(SPELL_SABER_LASH_IMM,0))
            {
                TargetGUID[i] = pUnit->GetGUID();
                pUnit->CastSpell(pUnit, SPELL_TELEPORT_VISUAL, true);
                //DoTeleportPlayer(pUnit, X, Y, Z, pUnit->GetOrientation());
                pUnit->GetMotionMaster()->MovementExpired();
                reinterpret_cast<Player*>(pUnit)->Relocate(X, Y, Z);
                reinterpret_cast<Player*>(pUnit)->TeleportTo(pUnit->GetMapId(), X, Y, Z, pUnit->GetOrientation(), TELE_TO_NOT_LEAVE_COMBAT);
            }
        }
    }
    
    bool TryDoCast(Unit *victim, uint32 spellId, bool triggered = false)
    {
        if(m_creature->IsNonMeleeSpellCasted(false)) return false;

        DoCast(victim,spellId,triggered);
        return true;
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if(((m_creature->GetHealth()*100 / m_creature->GetMaxHealth()) < 10) && !Enraged)
        {
            Enraged = true;
            DoCast(m_creature, SPELL_ENRAGE, true);
            DoScriptText(SAY_ENRAGE, m_creature);
        }
        
        // Only check the last 3 teleported players
        if (CheckPlayersUndermapTimer < diff) {
            for (int i = 0; i < 3; i++) {
                if (Player* plr = Unit::GetPlayer(TargetGUID[i])) {
                    float z = plr->GetPositionZ();
                    if (z < 189)      // Player seems to be undermap (ugly hack, isn't it ?)
                        DoTeleportPlayer(plr, 945.6173, 198.3479, 192.00, 4.674);
                }
            }
            
            CheckPlayersUndermapTimer = 5000;
        }else CheckPlayersUndermapTimer -= diff;

        //Randomly cast one beam.
        if(BeamTimer < diff)
        {
            Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0);
            if(!target || !target->isAlive())
                return;

            BeamTimer = 9000;

            switch(CurrentBeam)
            {
                case 0:
                    if (m_creature->getVictim() && target != m_creature->getVictim())   // prevent casting this one on the main tank
                        DoCast(target, SPELL_BEAM_SINISTER);
                    break;
                case 1:
                    DoCast(target, SPELL_BEAM_VILE);
                    break;
                case 2:
                    DoCast(target, SPELL_BEAM_WICKED);
                    break;
                case 3:
                    DoCast(target, SPELL_BEAM_SINFUL);
                    break;
            }
            //BeamCount++;
            uint32 Beam = CurrentBeam;
            //if(BeamCount > 3)
            while(CurrentBeam == Beam)
                CurrentBeam = rand()%3;

        }else BeamTimer -= diff;

        // Random Prismatic Shield every 15 seconds.
        if(PrismaticShieldTimer < diff)
        {
            uint32 random = rand()%6;
            if(PrismaticAuras[random])
            {
                if (LastPrismaticAura)  // Don't remove before first aura
                    m_creature->RemoveAurasDueToSpell(LastPrismaticAura);
                DoCast(m_creature, PrismaticAuras[random]);
                LastPrismaticAura = PrismaticAuras[random];
            }
            PrismaticShieldTimer = 15000;
        }else PrismaticShieldTimer -= diff;

        // Select 3 random targets (can select same target more than once), teleport to a random location then make them cast explosions until they get away from each other.
        if(FatalAttractionTimer < diff)
        {
            //ExplosionCount = 0;

            TeleportPlayers();

            DoScriptText(RAND(SAY_SPELL2,SAY_SPELL3), m_creature);
            FatalAttractionExplodeTimer = 2000;
            FatalAttractionTimer = 30000;
        }else FatalAttractionTimer -= diff;

        if(FatalAttractionExplodeTimer < diff)
        {
            Player* targets[3];
            for(uint8 i = 0; i < 3; ++i)
            {
                if(TargetGUID[i])
                    targets[i] = Player::GetPlayer(TargetGUID[i]);
                else
                    targets[i] = NULL;
            }

            if(targets[0] && targets[0]->isAlive())
            {
                bool isNear = false;
                if(targets[1] && targets[1]->isAlive() && targets[0]->GetDistance2d(targets[1]) < 25)
                    isNear = true;

                if(!isNear)
                    if(targets[2] && targets[2]->isAlive() && targets[0]->GetDistance2d(targets[2]) < 25)
                        isNear = true;
                
                if(isNear)
                    targets[0]->CastSpell(targets[0],SPELL_ATTRACTION,true);
                else
                {
                    targets[0]->RemoveAurasDueToSpell(SPELL_ATTRACTION_VIS);
                    TargetGUID[0] = 0;
                    targets[0] = NULL;
                }
            }

            
            if(targets[1] && targets[1]->isAlive())
            {
                bool isNear = false;
                if(targets[0] && targets[0]->isAlive() && targets[1]->GetDistance2d(targets[0]) < 25)
                    isNear = true;

                if(!isNear)
                    if(targets[2] && targets[2]->isAlive() && targets[1]->GetDistance2d(targets[2]) < 25)
                        isNear = true;
                
                if(isNear)
                    targets[1]->CastSpell(targets[1],SPELL_ATTRACTION,true);
                else
                {
                    targets[1]->RemoveAurasDueToSpell(SPELL_ATTRACTION_VIS);
                    TargetGUID[1] = 0;
                    targets[1] = NULL;
                }
            }

            if(targets[2] && targets[2]->isAlive())
            {
                bool isNear = false;
                if(targets[0] && targets[0]->isAlive() && targets[2]->GetDistance2d(targets[0]) < 25)
                    isNear = true;

                if(!isNear)
                    if(targets[1] && targets[1]->isAlive() && targets[2]->GetDistance2d(targets[1]) < 25)
                        isNear = true;

                if(isNear)
                    targets[2]->CastSpell(targets[1],SPELL_ATTRACTION,true);
                else
                {
                    targets[2]->RemoveAurasDueToSpell(SPELL_ATTRACTION_VIS);
                    TargetGUID[2] = 0;
                    targets[2] = NULL;
                }
            }

            bool allClear = true;
            for(uint8 i = 0; i < 3; i++)
            {
                if(TargetGUID[i] != 0)
                    allClear = false;
            }

            if(allClear)
                FatalAttractionExplodeTimer = 60000;
            else
                FatalAttractionExplodeTimer = 1000;

        }else FatalAttractionExplodeTimer -= diff;

        if(ShriekTimer < diff)
        {
            if(TryDoCast(m_creature->getVictim(), SPELL_SILENCING_SHRIEK))
                ShriekTimer = 25000+rand()%10 * 1000;
        }else ShriekTimer -= diff;

        if(SaberTimer < diff)
        {
            DoCast(m_creature->getVictim(), SPELL_SABER_LASH);
            SaberTimer = 25000+rand()%10 * 1000;
        }else SaberTimer -= diff;

        //Enrage
        if(!m_creature->HasAura(SPELL_BERSERK, 0))
            if(EnrageTimer < diff)
        {
            DoCast(m_creature, SPELL_BERSERK);
            DoScriptText(SAY_ENRAGE, m_creature);
        }else EnrageTimer -= diff;

        //Random taunts
        if(RandomYellTimer < diff)
        {
            DoScriptText(RAND(SAY_TAUNT1,SAY_TAUNT2,SAY_TAUNT3), m_creature);
            RandomYellTimer = 60000 + rand()%91 * 1000;
        }else RandomYellTimer -= diff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_shahraz(Creature *_Creature)
{
    return new boss_shahrazAI (_Creature);
}

void AddSC_boss_mother_shahraz()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_mother_shahraz";
    newscript->GetAI = &GetAI_boss_shahraz;
    newscript->RegisterSelf();
}

