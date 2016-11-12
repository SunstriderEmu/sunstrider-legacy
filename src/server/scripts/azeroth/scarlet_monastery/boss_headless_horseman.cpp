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
SDName: Boss_Headless_Horseman
SD%Complete:
SDComment:
SDCategory: Scarlet Monastery
EndScriptData */


#include "SpellMgr.h"
#include "def_scarlet_monastery.h"

enum HeadlessHorsemanSays {
    SAY_ENTRANCE                = -1189001,
    SAY_REJOINED                = -1189002,
    SAY_LOST_HEAD               = -1189003,
    SAY_CONFLAGRATION           = -1189004,
    SAY_SPROUTING_PUMPKINS      = -1189005,
    SAY_PLAYER_DEATH            = -1189006,
    SAY_DEATH                   = -1189007
};

uint32 RandomLaugh[] = {11965, 11975, 11976};

enum HeadlessHorsemanEntries {
    HH_MOUNTED                  = 23682,
    HH_UNHORSED                 = 23800,
    HEAD                        = 23775,
    PULSING_PUMPKIN             = 23694,
    PUMPKIN_FIEND               = 23545,
    HELPER                      = 23686,
    WISP_INVIS                  = 24034
};

enum HeadlessHorsemanSpells {
    SPELL_CLEAVE                = 42587,
    SPELL_CONFLAGRATION         = 42380,       //Phase 2, can't find real spell(Dim Fire?)
    //SPELL_CONFL_SPEED         = 22587,       //8% increase speed, value 22587 from SPELL_CONFLAGRATION mains that spell?
    SPELL_SUMMON_PUMPKIN        = 42394,

    SPELL_WHIRLWIND             = 43116,
    SPELL_IMMUNE                = 42556,
    SPELL_BODY_REGEN            = 42403,
    SPELL_CONFUSE               = 43105,

    SPELL_FLYING_HEAD           = 42399,       //visual flying head
    SPELL_HEAD                  = 42413,       //visual buff, "head"
    SPELL_HEAD_IS_DEAD          = 42428,       //at killing head, Phase 3

    SPELL_PUMPKIN_AURA          = 42280,
    SPELL_PUMPKIN_AURA_GREEN    = 42294,
    SPELL_SQUASH_SOUL           = 42514,
    SPELL_SPROUTING             = 42281,
    SPELL_SPROUT_BODY           = 42285,

    //Effects
    SPELL_RHYME_BIG             = 42909,
    //SPELL_RHYME_SMALL         = 42910,
    SPELL_HEAD_SPEAKS           = 43129,
    SPELL_HEAD_LANDS            = 42400,
    SPELL_BODY_FLAME            = 42074,
    SPELL_HEAD_FLAME            = 42971,
    //SPELL_ENRAGE_VISUAL       = 42438,      // he uses this spell?
    SPELL_WISP_BLUE             = 42821,
    SPELL_WISP_FLIGHT_PORT      = 42818,
    //SPELL_WISP_INVIS          = 42823,
    SPELL_SMOKE                 = 42355,
    SPELL_DEATH                 = 42566       //not correct spell
};

struct Locations
{
    float x, y, z;
};

static Locations FlightPoint[] =
{
    {1754.00, 1346.00, 17.50},
    {1765.00, 1347.00, 19.00},
    {1784.00, 1346.80, 25.40},
    {1803.30, 1347.60, 33.00},
    {1824.00, 1350.00, 42.60},
    {1838.80, 1353.20, 49.80},
    {1852.00, 1357.60, 55.70},
    {1861.30, 1364.00, 59.40},
    {1866.30, 1374.80, 61.70},
    {1864.00, 1387.30, 63.20},
    {1854.80, 1399.40, 64.10},
    {1844.00, 1406.90, 64.10},
    {1824.30, 1411.40, 63.30},
    {1801.00, 1412.30, 60.40},
    {1782.00, 1410.10, 55.50},
    {1770.50, 1405.20, 50.30},
    {1765.20, 1400.70, 46.60},
    {1761.40, 1393.40, 41.70},
    {1759.10, 1386.70, 36.60},
    {1757.80, 1378.20, 29.00},
    {1758.00, 1367.00, 19.51}
};

static Locations Spawn[] =
{
    {1776.27,1348.74,19.20},        //spawn point for pumpkin shrine mob
    {1765.28,1347.46,17.55}     //spawn point for smoke
};

struct Summon
{
    const std::string text;
};

static Summon Text[] =
{
    {"Lève-toi, cavalier..."},
    {"Ton heure va sonner..."},
    {"La mort, tu connais bien..."},
    {"Voici l'heure de ta fin !"}
};

struct mob_wisp_invisAI : public ScriptedAI
{
    mob_wisp_invisAI(Creature* c) : ScriptedAI(c)
    {
        Creaturetype = delay = spell = spell2 = 0;
    }

    uint32 Creaturetype;
    uint32 delay;
    uint32 spell;
    uint32 spell2;
    
    void Reset() override {}
    
    void EnterCombat(Unit* who) override {}
    
    void SetType(uint32 _type)
    {
        Creaturetype = _type;
        switch(Creaturetype)
        {
        case 1:
            spell = SPELL_PUMPKIN_AURA_GREEN;
            break;
        case 2:
            delay = 15000;
            spell = SPELL_BODY_FLAME;
            spell2 = SPELL_DEATH;
            break;
        case 3:
            delay = 15000; 
            spell = SPELL_SMOKE;
            break;
        case 4:
            delay = 7000;
            spell2 = SPELL_WISP_BLUE;
            break;
        default:
            break;
        }
        
        if (spell)
            DoCast(me, spell);
    }

    void SpellHit(Unit* caster, const SpellInfo *spell)
    override {
        if (spell->Id == SPELL_WISP_FLIGHT_PORT && Creaturetype == 4)
            me->SetDisplayId(2027);
    }

    void MoveInLineOfSight(Unit *who)
    override {
        if (!who || Creaturetype != 1 || me->CanAttack(who) != CAN_ATTACK_RESULT_OK)
            return;

        if (me->GetDistance2d(who) < 0.1 && !who->HasAuraEffect(SPELL_SQUASH_SOUL,0))
            DoCast(who, SPELL_SQUASH_SOUL);
    }

    void UpdateAI(const uint32 diff)
    override {
        if(delay) {
            if (delay <= diff) {
                me->RemoveAurasDueToSpell(SPELL_SMOKE);
                if (spell2)
                    DoCast(me,spell2);

                delay = 0;
            }
            else
                delay -= diff;
        }
    }
};

struct mob_headAI : public ScriptedAI
{
    mob_headAI(Creature* c) : ScriptedAI(c) {}

    uint64 bodyGUID;

    uint32 Phase;
    uint32 laugh;
    uint32 wait;

    bool withbody;
    bool die;

    void Reset()
    override {
        Phase = 0;
        bodyGUID = 0;
        die = false;
        withbody = true;
        wait = 1000;
        laugh = 15000 + rand()%16 * 1000;
    }

    void EnterCombat(Unit* who) override {}

    void SaySound(int32 textEntry, Unit* target = nullptr)
    {
        DoScriptText(textEntry, me, target);
        //DoCast(me, SPELL_HEAD_SPEAKS, true);
        Creature *speaker = DoSpawnCreature(HELPER, 0, 0, 0, 0, TEMPSUMMON_TIMED_DESPAWN, 1000);
        if (speaker)
            speaker->CastSpell(speaker, SPELL_HEAD_SPEAKS, false);

        laugh += 3000;
    }
    
    /*void EnterEvadeMode(EvadeReason why )
    {
        if (bodyGUID) {
            if (Creature* body = ObjectAccessor::GetCreature((*me), bodyGUID))
                body->AI()->EnterEvadeMode();
        }
        ScriptedAI::EnterEvadeMode(why);
    }*/

    void DamageTaken(Unit* doneBy, uint32& damage)
    override {
        if (withbody)
            return;

        switch(Phase)
        {
        case 1:
            if (((me->GetHealth() - damage)*100)/me->GetMaxHealth() < 67)
                Disappear();
                
            break;
        case 2:
            if (((me->GetHealth() - damage)*100)/me->GetMaxHealth() < 34)
                Disappear();
            
            break;
        case 3:
            if (damage >= me->GetHealth()) {
                die = true;
                withbody = true;
                wait = 300;
                damage = me->GetHealth() - me->GetMaxHealth()/100;
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                me->StopMoving();
                //me->GetMotionMaster()->MoveIdle();
                DoCast(me, SPELL_HEAD_IS_DEAD);
            }
            break;
        default:
            break;
        }
        
        /*if (bodyGUID) { // Handled in body script
            Creature *body = ObjectAccessor::GetCreature((*me), bodyGUID);
            if (body && body->GetHealth() == body->GetMaxHealth())
                Disappear();
        }*/
    }

    void SpellHit(Unit* caster, const SpellInfo* spell)
    override {
        if (!withbody)
            return;

        if (spell->Id == SPELL_FLYING_HEAD)
        {
            if (Phase < 3)
                ++Phase;
            else
                Phase = 3;

            withbody = false;
            if(!bodyGUID)
                bodyGUID = caster->GetGUID();

            me->RemoveAllAuras();
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            DoCast(me, SPELL_HEAD_LANDS, true);
            DoCast(me, SPELL_HEAD, false);
            SaySound(SAY_LOST_HEAD);
            me->GetMotionMaster()->Clear(false);
            me->GetMotionMaster()->MoveChase(caster->GetVictim());
        }
    }
    
    void Disappear(); // We must set returned = true(this will prevent from "body calls head" while head flying to body), see function below
    
    void UpdateAI(const uint32 diff)
    override {
        if (!withbody) {
            if (wait <= diff) {
                wait = 1000;
                if (!me->GetVictim())
                    return;
                me->GetMotionMaster()->Clear(false);
                me->GetMotionMaster()->MoveFleeing(me->GetVictim());
            }
            else
                wait -= diff;

            if (laugh <= diff){
                laugh = 15000 + (rand()%16)*IN_MILLISECONDS;
                DoPlaySoundToSet(me, RandomLaugh[rand()%3]);
                //DoCast(me,SPELL_HEAD_SPEAKS,true); //this spell remove buff "head"
                Creature *speaker = DoSpawnCreature(HELPER, 0, 0, 0, 0, TEMPSUMMON_TIMED_DESPAWN, 1000);
                if (speaker)
                    speaker->CastSpell(speaker, SPELL_HEAD_SPEAKS, false);

                me->TextEmote("rit.",nullptr);
            } else laugh -= diff;

        }
        else {
            if (die) {
                if (wait <= diff) {
                    die = false;
                    Unit *body = ObjectAccessor::GetUnit((*me),bodyGUID);
                    if (body)
                        body->DealDamage(body, body->GetMaxHealth(), nullptr, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, nullptr, false);

                    me->SetDeathState(JUST_DIED);
                }
                else
                    wait -= diff;
            }
        }
    }
};

struct boss_headless_horsemanAI : public ScriptedAI
{
    boss_headless_horsemanAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = ((InstanceScript*)c->GetInstanceScript());
    }

    InstanceScript *pInstance;

    uint64 headGUID;
    uint64 playerGUID;

    uint32 Phase;
    uint32 id;
    uint32 count;
    uint32 say_timer;

    uint32 conflagrate;
    uint32 summonadds;
    uint32 cleave;
    uint32 regen;
    uint32 whirlwind;
    uint32 laugh;
    uint32 burn;

    bool withhead;
    bool returned;
    bool IsFlying;
    bool wp_reached;
    bool burned;

    void Reset()
    override {
        Phase = 1;
        conflagrate = 15000;
        summonadds = 15000;
        laugh = 16000 + rand()%5 * 1000;
        cleave = 2000;
        regen = 1000;
        burn = 6000;
        count = 0;
        say_timer = 3000;

        withhead = true;
        returned = true;
        burned = false;
        IsFlying = false;
        DoCast(me, SPELL_HEAD);

        if (headGUID){
            Unit* Head = ObjectAccessor::GetUnit((*me), headGUID);
            if (Head) {
                Head->SetVisibility(VISIBILITY_OFF);
                Head->SetDeathState(JUST_DIED);
            }

            headGUID = 0;
        }

        if (pInstance)
            pInstance->SetData(DATA_HORSEMAN_EVENT, NOT_STARTED);
            
        me->SetName("Cavalier sans tête");
    }

    void FlyMode()
    {
        me->SetVisibility(VISIBILITY_OFF);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        me->SetDisableGravity(true);
        me->SetSpeedRate(MOVE_WALK, 5.0f, true);
        wp_reached = false;
        count = 0;
        say_timer = 3000;
        id = 0;
        Phase = 0;
    }

    void MovementInform(uint32 type, uint32 i)
    override {
        if (type != POINT_MOTION_TYPE || !IsFlying)
            return;

        if (i != id)
            return;

        wp_reached = true;

        switch (id) {
        case 0:
            me->SetVisibility(VISIBILITY_ON);
            break;
        case 1:
        {
            Creature *smoke = me->SummonCreature(HELPER, Spawn[1].x, Spawn[1].y, Spawn[1].z, 0, TEMPSUMMON_TIMED_DESPAWN, 20000);
            if (smoke)
                ((mob_wisp_invisAI*)smoke->AI())->SetType(3);

            DoCast(me, SPELL_RHYME_BIG);
            break;
        }
        case 6:
            if (pInstance)
                pInstance->SetData(GAMEOBJECT_PUMPKIN_SHRINE, 0);   //hide gameobject

            break;
        case 19:
            me->SetDisableGravity(false);
            break;
        case 20:
        {
            Phase = 1;
            IsFlying = false;
            wp_reached = false;
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            SaySound(SAY_ENTRANCE);
            Unit *plr = ObjectAccessor::GetUnit((*me),playerGUID);
            if (plr)
                AttackStart(plr);

            me->SetHomePosition(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation());

            break;
        }
        }
        ++id;
    }

    void EnterCombat(Unit* who)
    override {
        if(pInstance)
            pInstance->SetData(DATA_HORSEMAN_EVENT, IN_PROGRESS);

        DoZoneInCombat();
    }
    

    void MoveInLineOfSight(Unit* who)
    override {
        if (withhead && Phase != 0)
            ScriptedAI::MoveInLineOfSight(who);
    }
    
    void KilledUnit(Unit* victim)
    override {
        if (victim->GetTypeId() == TYPEID_PLAYER) {
            if (withhead)
                SaySound(SAY_PLAYER_DEATH);
            else {          // may be possible when player dies from conflagration
                Creature *Head = ObjectAccessor::GetCreature((*me), headGUID);
                if (Head)
                    ((mob_headAI*)Head->AI())->SaySound(SAY_PLAYER_DEATH);
            }
        }
    }

    void SaySound(int32 textEntry, Unit *target = nullptr)
    {
        DoScriptText(textEntry, me, target);
        laugh += 4000;
    }

    Player* SelectRandomPlayer(float range = 0.0f, bool checkLoS = true)
    {
        Map *map = me->GetMap();
        if (!map->IsDungeon())
            return nullptr;

        Map::PlayerList const& playerList = map->GetPlayers();
        Map::PlayerList::const_iterator i;
        if (playerList.isEmpty())
            return nullptr;

        std::list<Player*> temp;
        std::list<Player*>::iterator j;

        for (const auto & i : playerList)
            if ((me->IsWithinLOSInMap(i.GetSource()) || !checkLoS) && me->GetVictim() != i.GetSource() &&
                me->IsWithinDistInMap(i.GetSource(), range) && i.GetSource()->IsAlive() && i.GetSource()->IsAttackableByAOE())
                temp.push_back(i.GetSource());

        if (temp.size()) {
            j = temp.begin();
            advance(j, rand()%temp.size());
            return (*j);
        }

        return nullptr;
    }

    void SpellHitTarget(Unit* unit, const SpellInfo* spell)
    override {
        if (spell->Id == SPELL_CONFLAGRATION) {
            if (unit->HasAuraEffect(SPELL_CONFLAGRATION))
                SaySound(SAY_CONFLAGRATION,unit);
        }
    }

    void JustDied(Unit* killer)
    override {
        me->StopMoving();
        //me->GetMotionMaster()->MoveIdle();    test
        SaySound(SAY_DEATH);

        Creature* flame = DoSpawnCreature(HELPER, 0, 0, 0, 0, TEMPSUMMON_TIMED_DESPAWN, 60000);
        if (flame)
            flame->CastSpell(flame, SPELL_BODY_FLAME, false);

        Creature* wisp = DoSpawnCreature(WISP_INVIS, 0, 0, 0, 0, TEMPSUMMON_TIMED_DESPAWN, 60000);
        if (wisp)
            ((mob_wisp_invisAI*)wisp->AI())->SetType(4);

        if (pInstance)
            pInstance->SetData(DATA_HORSEMAN_EVENT, DONE);
            
        Unit* Head = ObjectAccessor::GetUnit((*me), headGUID);
        if (Head && Head->IsAlive())
            Head->ToCreature()->DisappearAndDie();
            
        Map *map = me->GetMap();
        if (!map->IsDungeon())
            return;
            
        Map::PlayerList const& playerList = map->GetPlayers();
        Map::PlayerList::const_iterator i;
        if (playerList.isEmpty())
            return;
            
        for (const auto & i : playerList) {
            if (Player* plr = i.GetSource())
                plr->CombatStop();
        }
        
        me->SummonCreature(23904, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, 600000);
    }

    void SpellHit(Unit *caster, const SpellInfo* spell)
    override {
        if (withhead)
            return;

        if (spell->Id == SPELL_FLYING_HEAD) {
            if (Phase < 3)
                Phase++;
            else
                Phase = 3;

            withhead = true;
            me->RemoveAllAuras();
            me->SetName("Cavalier sans tête");
            me->SetHealth(me->GetMaxHealth());
            SaySound(SAY_REJOINED);
            DoCast(me,SPELL_HEAD);
            caster->GetMotionMaster()->Clear(false);
            caster->GetMotionMaster()->MoveFollow(me, 6, rand()%6);
            //DoResetThreat();//not sure if need
            std::list<HostileReference*>::iterator itr;
            for (itr = caster->getThreatManager().getThreatList().begin(); itr != caster->getThreatManager().getThreatList().end(); ++itr) {
                Unit* pUnit = ObjectAccessor::GetUnit((*me), (*itr)->getUnitGuid());
                if (pUnit && pUnit->IsAlive() && pUnit != caster)
                    me->AddThreat(pUnit, caster->getThreatManager().getThreat(pUnit));
            }
        }
    }

    void DamageTaken(Unit* doneBy, uint32& damage)
    override {
        if (damage >= me->GetHealth() && withhead) {
            withhead = false;
            returned = false;
            if (Phase < 3)
                damage = 0;
            me->RemoveAllAuras();
            //me->SetName("Cavalier sans tête, à pied");

            if (!headGUID)
                headGUID = DoSpawnCreature(HEAD, rand()%6, rand()%6, 0, 0, TEMPSUMMON_DEAD_DESPAWN, 0)->GetGUID();

            Unit* Head = ObjectAccessor::GetUnit((*me), headGUID);
            if (Head && Head->IsAlive()) {
                Head->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                //Head->CastSpell(Head,SPELL_HEAD_INVIS,false);
                me->InterruptNonMeleeSpells(false);
                DoCast(me, SPELL_IMMUNE, true);
                DoCast(me, SPELL_BODY_REGEN, true);
                me->CastSpell(Head, SPELL_FLYING_HEAD, true);
                DoCast(me, SPELL_CONFUSE, false);                     //test
                doneBy->ProcDamageAndSpell(me, PROC_FLAG_KILL_AND_GET_XP, PROC_FLAG_KILLED, PROC_EX_NONE, 0);
                whirlwind = 4000 + (rand()%5)*IN_MILLISECONDS;
                regen = 0;
            }
            
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        }
        else if (!withhead)
            damage = 0;
    }

    void UpdateAI(const uint32 diff)
    override {
        me->AddUnitState(UNIT_STATE_IGNORE_PATHFINDING);

        if (withhead) {
            if (Phase == 0) {
                if (!IsFlying) {
                    if (say_timer <= diff) {
                        say_timer = 3000;
                        Player* plr = SelectRandomPlayer(100.0f, false);
                        if (count < 3) {
                            if(plr)
                                plr->Say(Text[count].text,LANG_UNIVERSAL);
                        }
                        else {
                            DoCast(me, SPELL_RHYME_BIG);
                            if (plr) {
                                plr->Say(Text[count].text,LANG_UNIVERSAL);
                                plr->HandleEmoteCommand(ANIM_EMOTE_SHOUT);
                            }

                            wp_reached = true;
                            IsFlying = true;
                            count = 0;
                            return;
                        }
                        ++count;
                    }
                    else
                        say_timer -= diff;
                }
                else {
                    if (wp_reached) {
                        wp_reached = false;
                        me->GetMotionMaster()->Clear(false);
                        me->GetMotionMaster()->MovePoint(id, FlightPoint[id].x, FlightPoint[id].y, FlightPoint[id].z);
                    }
                }

                return;
            }
            else if (Phase == 1 || Phase == 2 || Phase == 3) {
                if (!burned) {
                    if (burn <= diff) {
                        if (Creature* flame = me->SummonCreature(HELPER, Spawn[0].x, Spawn[0].y, Spawn[0].z, 0, TEMPSUMMON_TIMED_DESPAWN, 17000))
                            ((mob_wisp_invisAI*)flame->AI())->SetType(2);

                        burned = true;
                    }
                    else
                        burn -= diff;
                }
            }
            if (Phase == 2 || Phase == 3) {
                if (conflagrate < diff)
                {
                    Unit *plr = (Unit*)SelectRandomPlayer(30.0f);
                    if (plr)
                        me->CastSpell(plr,SPELL_CONFLAGRATION,false);
                    conflagrate = 10000 + rand()%7 * 1000;
                }
                else
                    conflagrate -= diff;
            }
            if (Phase == 3) {
                if (summonadds <= diff) {
                    me->InterruptNonMeleeSpells(false);
                    DoCast(me, SPELL_SUMMON_PUMPKIN);
                    SaySound(SAY_SPROUTING_PUMPKINS);
                    summonadds = 25000 + rand()%11 * 1000;
                }
                else
                    summonadds -= diff;
            }

            if (laugh <= diff) {
                laugh = 11000 + rand()%12 * 1000;
                me->TextEmote("rit.", nullptr);
                DoPlaySoundToSet(me, RandomLaugh[rand()%3]);
            }
            else
                laugh -= diff;

            if (UpdateVictim()) {
                DoMeleeAttackIfReady();
                if (cleave <= diff) {
                    DoCast(me->GetVictim(),SPELL_CLEAVE);
                    cleave = 2000 * (1 + rand()%3);       //1 cleave per 2.0-6.0sec
                }
                else
                    cleave -= diff;
            }

        }
        else {
            if (regen <= diff) {
                regen = 1000;                   //"body calls head"
                if (me->GetHealth()/me->GetMaxHealth() == 1 && !returned)
                {
                    if (Phase > 1)
                        --Phase;
                    else
                        Phase = 1;

                    Creature* Head = ObjectAccessor::GetCreature((*me), headGUID);
                    if (Head && Head->IsAlive())
                    {
                        ((mob_headAI*)Head->AI())->Phase = Phase;
                        ((mob_headAI*)Head->AI())->Disappear();
                    }
                    return;
                }
            }
            else
                regen -= diff;

            if (whirlwind <= diff) {
                whirlwind = 4000 + rand()%5 * 1000;
                if (rand()%2) {
                    me->RemoveAurasDueToSpell(SPELL_CONFUSE);
                    DoCast(me, SPELL_WHIRLWIND, true);
                    DoCast(me, SPELL_CONFUSE);
                }
                else
                    me->RemoveAurasDueToSpell(SPELL_WHIRLWIND);
            }
            else
                whirlwind -= diff;
                
            if (!UpdateVictim())
                return;
        }
    }
};

void mob_headAI::Disappear()
{
    if (withbody)
        return;

    if (bodyGUID) {
        Creature *body = ObjectAccessor::GetCreature((*me), bodyGUID);
        if (body && body->IsAlive()) {
            withbody = true;
            me->RemoveAllAuras();
            body->RemoveAurasDueToSpell(SPELL_IMMUNE);//hack, SpellHit doesn't calls if body has immune aura
            body->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            DoCast(body, SPELL_FLYING_HEAD);
            me->SetHealth(me->GetMaxHealth());
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            me->GetMotionMaster()->MoveIdle();
            ((boss_headless_horsemanAI*)body->AI())->returned = true;
        }
    }
}

struct mob_pulsing_pumpkinAI : public ScriptedAI
{
    mob_pulsing_pumpkinAI(Creature* c) : ScriptedAI(c) {}

    bool sprouted;
    uint64 debuffGUID;

    void Reset()
    override {
        float x, y, z;
        me->GetPosition(x, y, z);   //this visual aura some under ground
        me->Relocate(x, y, z + 0.35f);
        Despawn();
        
        Creature* debuff = DoSpawnCreature(HELPER, 0, 0, 0, 0, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 14500);
        if (debuff) {
            debuff->SetDisplayId(me->GetDisplayId());
            debuff->CastSpell(debuff, SPELL_PUMPKIN_AURA_GREEN, false);
            ((mob_wisp_invisAI*)debuff->AI())->SetType(1);
            debuffGUID = debuff->GetGUID();
        }

        sprouted = false;
        DoCast(me, SPELL_PUMPKIN_AURA, true);
        DoCast(me, SPELL_SPROUTING);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_STUNNED);
    }

    void EnterCombat(Unit* who)override {}

    void SpellHit(Unit* caster, const SpellInfo* spell)
    override {
        if (spell->Id == SPELL_SPROUTING) {
            sprouted = true;
            me->RemoveAllAuras();
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_STUNNED);
            DoCast(me, SPELL_SPROUT_BODY, true);
            me->UpdateEntry(PUMPKIN_FIEND);
            AttackStart(me->GetVictim());
        }
    }

    void Despawn()
    {
        if (!debuffGUID)
            return;

        Unit* debuff = ObjectAccessor::GetUnit((*me), debuffGUID);
        if (debuff) {
            debuff->SetVisibility(VISIBILITY_OFF);
            debuffGUID = 0;
        }
    }

    void JustDied(Unit* killer)
    override {
        if (!sprouted)
            Despawn();
    }

    void MoveInLineOfSight(Unit* who)
    override {
        if (!who || me->CanAttack(who) != CAN_ATTACK_RESULT_OK || !me->IsHostileTo(who) || me->GetVictim())
            return;

        me->AddThreat(who, 0.0f);
        if(sprouted)
            AttackStart(who);
    }

    void UpdateAI(const uint32 diff)
    override {
        if (sprouted) {
            if (UpdateVictim())
                DoMeleeAttackIfReady();
        }
    }
};

class LooselyTurnedSoil : public GameObjectScript
{
public:
    LooselyTurnedSoil() : GameObjectScript("go_loosely_turned_soil")
    {}

    bool OnQuestReward(Player* player, GameObject* go, Quest const* quest, uint32 opt) override
    {
        if (quest->GetQuestId() == 11405) {
            if (Creature *horseman = go->SummonCreature(HH_MOUNTED, FlightPoint[20].x, FlightPoint[20].y, FlightPoint[20].z, 0, TEMPSUMMON_MANUAL_DESPAWN, 0)) {
                ((boss_headless_horsemanAI*)horseman->AI())->playerGUID = player->GetGUID();
                ((boss_headless_horsemanAI*)horseman->AI())->FlyMode();
            }
        }

        return true;
    }
};


CreatureAI* GetAI_mob_head(Creature* creature)
{
    return new mob_headAI(creature);
}

CreatureAI* GetAI_boss_headless_horseman(Creature* creature)
{
    return new boss_headless_horsemanAI (creature);
}

CreatureAI* GetAI_mob_pulsing_pumpkin(Creature* creature)
{
    return new mob_pulsing_pumpkinAI (creature);
}

CreatureAI* GetAI_mob_wisp_invis(Creature* creature)
{
    return new mob_wisp_invisAI (creature);
}

void AddSC_boss_headless_horseman()
{
    OLDScript *newscript;

    newscript = new OLDScript;
    newscript->Name="boss_headless_horseman";
    newscript->GetAI = &GetAI_boss_headless_horseman;
    sScriptMgr->RegisterOLDScript(newscript);

    newscript = new OLDScript;
    newscript->Name="mob_head";
    newscript->GetAI = &GetAI_mob_head;
    sScriptMgr->RegisterOLDScript(newscript);

    newscript = new OLDScript;
    newscript->Name="mob_pulsing_pumpkin";
    newscript->GetAI = &GetAI_mob_pulsing_pumpkin;
    sScriptMgr->RegisterOLDScript(newscript);

    newscript = new OLDScript;
    newscript->Name="mob_wisp_invis";
    newscript->GetAI = &GetAI_mob_wisp_invis;
    sScriptMgr->RegisterOLDScript(newscript);

    new LooselyTurnedSoil();
}
