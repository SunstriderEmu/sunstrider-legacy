#include "SimpleCooldown.h"

// Creature and visual
#define ID_MOB_DREAM_FOG                15224
#define VISUAL_SPELL_ID_DREAM_FOG       42344 // Flamme d'archilol pour l'instant

// Spell
#define SPELL_TAIL_SWEEP                38737 // A priori scripté
#define SPELL_NOXIOUS_BREATH            24818 // A priori scripté
#define SPELL_SLEEP_DREAM_FOG           24777 // Peut être augmenter la portée de l'aura
#define SPELL_MARK_OF_NATURE            25040
#define SPELL_AURA_OF_NATURE_STUN       25043
#define SPELL_TELEPORT_IN_FRONT         24776

// Timers
#define TIMER_TAIL_SWEEP                5000 
#define TIMER_NOXIOUS_BREATH            8000
#define TIMER_SEEPING_FOG               45000 
#define TIMER_DESPAWN_DREAM_FOG         30000 

//RandomRange timer
#define RANDOM_RANGE_TAIL_SWEEP         2000 
#define RANDOM_RANGE_NOXIOUS_BREATH     4000

#define MAX_DISTANCE_FROM_HOME          120


// Template Dragon of Nightmare

struct DragonOfNightmareAI_template : public ScriptedAI
{
    bool initialPositionInitialised;
    float initialX, initialY,initialZ;
    
    Creature* DreamFog1;
    Creature* DreamFog2;
    
    int NumSpell25PrecentLeft;
    int lowHpYellLeft;
    
    SimpleCooldown* SCDTailSweep;
    SimpleCooldown* SCDNoxiousBreath;
    SimpleCooldown* SCDSeepingFog;
    
    void EnterCombat(Unit *Who)
    override {
    }
    
    DragonOfNightmareAI_template(Creature *c) : ScriptedAI(c)
    {
        initialPositionInitialised=false;
        
        SCDTailSweep = new SimpleCooldown(TIMER_TAIL_SWEEP, TIMER_TAIL_SWEEP, true, RANDOM_RANGE_TAIL_SWEEP, false, false);
        SCDNoxiousBreath= new SimpleCooldown(TIMER_NOXIOUS_BREATH, TIMER_NOXIOUS_BREATH, true, RANDOM_RANGE_NOXIOUS_BREATH, false, false);
        SCDSeepingFog= new SimpleCooldown(TIMER_SEEPING_FOG);
        
        DreamFog1=nullptr;
        DreamFog2=nullptr;
    }

    void Instakill(Unit* Target)
    {
        Target->DealDamage(Target, Target->GetHealth(), nullptr, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, nullptr, false);
    }
    
    void Reset()
    override {
        if(!initialPositionInitialised)
        {
            initialPositionInitialised=true;
            me->GetPosition(initialX, initialY,initialZ);
        }
        
        SCDTailSweep->resetAtStart();
        SCDNoxiousBreath->resetAtStart();
        SCDSeepingFog->resetAtStart();
        NumSpell25PrecentLeft=3;
        lowHpYellLeft=1;
    }
    
    void JustDied(Unit *victim)
    override {
        DreamFog1=nullptr;
        DreamFog2=nullptr;
    }
    
    void CastSeepingFog()
    {        
        // et on lui lance un nuage DANS SA GUEULE MOTHERFUCK
        if(Creature* DreamFog1Unit = DoSpawnCreature(ID_MOB_DREAM_FOG, 0, 0, 0, 0, TEMPSUMMON_TIMED_DESPAWN, TIMER_DESPAWN_DREAM_FOG))
        {
            DreamFog1 = (Creature*) DreamFog1Unit;
            DreamFog1->SetFaction(me->GetFaction());
        }
        if(Creature* DreamFog2Unit = DoSpawnCreature(ID_MOB_DREAM_FOG, 0, 0, 0, 0, TEMPSUMMON_TIMED_DESPAWN, TIMER_DESPAWN_DREAM_FOG))
        {
            DreamFog2 = (Creature*) DreamFog2Unit;
            DreamFog2->SetFaction(me->GetFaction());
        }
    }
    
    void UpdateAI(const uint32 diff)
    override {
        if (!UpdateVictim())
            return;
        
        if(me->GetDistance(initialX, initialY, initialZ)>MAX_DISTANCE_FROM_HOME)
            EnterEvadeMode();
        
        
        checkIfSomeoneAffectedByMarkOfNatureOrIsTooFar();
        
        if(SCDTailSweep->CheckAndUpdate(diff))
            DoCast(me,SPELL_TAIL_SWEEP, true);
        if(SCDNoxiousBreath->CheckAndUpdate(diff))
            DoCast(me,SPELL_NOXIOUS_BREATH, true);
        if(SCDSeepingFog->CheckAndUpdate(diff))
            CastSeepingFog();
    }
    
    void KilledUnit(Unit* victim)
    override {
        if(victim)
            me->AddAura(SPELL_MARK_OF_NATURE, victim);
    }
    
    // Bordel mais permet un seul parcourt de liste au lieu de deux
    void checkIfSomeoneAffectedByMarkOfNatureOrIsTooFar()
    {
        //récupération de la liste d'aggro
        std::list<HostileReference*>& threatList = me->getThreatManager().getThreatList();
            if (threatList.empty())
                return;
        // On prends les cibles intéressante (unit et vivantes))
        std::list<Unit*> targets;
        for (auto & itr : threatList) 
        {
            Unit* unit = ObjectAccessor::GetUnit((*me), itr->getUnitGuid());
            if (unit && unit->IsAlive() && unit->HasAuraEffect(SPELL_MARK_OF_NATURE,0))
            {
                //DoCast(unit,SPELL_AURA_OF_NATURE_STUN,true);
                unit->CastSpell(unit,SPELL_AURA_OF_NATURE_STUN,true);
                unit->RemoveAura(SPELL_MARK_OF_NATURE,0,nullptr);

                if(me->GetDistance2d(unit->GetPositionX(),unit->GetPositionY())>100)
                    DoCast(unit,SPELL_TELEPORT_IN_FRONT,true);
            }
        }
        return;
    }
    
    bool shouldCast25PrecentSpell(int &NumSpellLeft)
    {
        if(NumSpellLeft==3 && me->IsBelowHPPercent(75))
        {
            NumSpellLeft--;
            return true;
        }
        
        if(NumSpellLeft==2 && me->IsBelowHPPercent(50))
        {
            NumSpellLeft--;
            return true;
        }
        
        if(NumSpellLeft==1 && me->IsBelowHPPercent(25))
        {
            NumSpellLeft--;
            return true;
        }
        return false;
    }
    
};

struct DreamFogAI : public ScriptedAI
{
    SimpleCooldown* SCDTimerUpdateMovement;
    SimpleCooldown* SCDSecondBeforeUpdateVictim;
    Unit* Target;
    bool StarUpdateVictimTimer;
    Creature* Dragon;
    
    DreamFogAI(Creature *c) : ScriptedAI(c)
    {
        SCDTimerUpdateMovement = new SimpleCooldown(1000);
        SCDSecondBeforeUpdateVictim = new SimpleCooldown(1000);
        Dragon=ObjectAccessor::GetCreature(*me,me->GetOwnerGUID());
        setTargetFromDragon();
    }
    void EnterCombat(Unit *who)override {}
    
    void Reset()
    override {
        SCDTimerUpdateMovement->reinitCD();
        SCDSecondBeforeUpdateVictim->reinitCD();
        Target = nullptr;
                
        // Jsais plus pouruquoi j'ai set tout ca mais jlai fait et ca marche :p
        me->SetFloatValue(UNIT_FIELD_BOUNDINGRADIUS, 0);
        me->SetFloatValue(UNIT_FIELD_COMBATREACH, 0);
        me->SetFloatValue(UNIT_FIELD_RANGEDATTACKTIME, 0);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        
        for (int i=0 ; i<10 ; i++)// Cast visual 10 times to make it more beatiful <3
            DoCast(me, SPELL_SLEEP_DREAM_FOG, true);
        //DoCast(me, VISUAL_SPELL_ID_NUAGE_ONIRIQUE, true);
    }
    
    bool UpdateVictim(bool evade)
    {
        // On change pas de victime, on est face à un esprit assez borné!
        return true;
    }
    
    void DoMeleeAttackIfReady() override
    {
        // Borné, mais pacifiste!
        return;
    }
    
    void Instakill(Unit* Target)
    {
        Target->DealDamage(Target, Target->GetHealth(), nullptr, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, nullptr, false);
    }
    
    void UpdateAI(const uint32 diff)
    override {   
        if(!Dragon )
        {
            Dragon=ObjectAccessor::GetCreature(*me,me->GetOwnerGUID());
            return;
        }
        
        if(Dragon->IsDead())
            Instakill(me);
        
        //me->AI()->AttackStart(Cible);
        if(SCDTimerUpdateMovement->CheckAndUpdate(diff))
                me->GetMotionMaster()->MoveFollow(Target,0,0);
        // Partie très moche a changer. Si l'esprit est trop proche (mais pas assez pour l'aura) alors on le téléporte sur le gars.
        if(me->GetDistance(Target->GetPositionX(),Target->GetPositionY(),Target->GetPositionZ())<3) // J'ai mis 3 car c'est petit et ca fait un coeur!
        {
            me->Relocate(Target->GetPositionX(),Target->GetPositionY(),Target->GetPositionZ());
            StarUpdateVictimTimer=true;
        }
        // Wait one second before death
        if(StarUpdateVictimTimer && Dragon)
        {
            if(SCDSecondBeforeUpdateVictim->CheckAndUpdate(diff))
            {
                setTargetFromDragon();
                StarUpdateVictimTimer=false;
            }
        }
    }

    void setTarget(Unit* target)
    {
        Target = target;
    }
    
    void setTargetFromDragon()
    {
        if(!Dragon)
            return;
        setTarget(((DragonOfNightmareAI_template*)Dragon->AI())->SelectTarget(SELECT_TARGET_RANDOM,0));
    }
};

CreatureAI* GetAI_DreamFog(Creature *_Creature)
{
    return new DreamFogAI (_Creature);
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////


// Lethon //

// Mobs and Visuals
#define ID_MOB_SHADOW_SPIRIT            15261
#define VISUAL_SPELL_SHADOW_SPIRIT      24809 // Apparement change aussi en selenien WTF???

// Yell
#define YELL_ON_AGGRO_LETHON            "Je peux sentir l'ombre corrompre vos coeurs. Il ne peut y avoir de répis pour les impurs. PÉRISSEZ!"
#define YELL_AT_PHASE_CHANGE_LETHON     "Vos âmes corrompues par les ombres seront ma force. Venez à moi pour restaurer mon pouvoir!"
#define YELL_AT_5_PRECENT_LIFE_LETHON   "Ce n'est pas... encore... fini! Il me reste... encore ... des forces..."
#define TELL_AT_DEATH_LETHON            "Non.. C'est impossible! Jamais l'ombre ne ... pourra... Arrrrrrrgh!"

// Spell
#define SPELL_AOE_SHADOWBOLT_LETHON     28407 // C'est pas le bon mais fonctionne bien
#define SPELL_STUN_POP_SPIRIT           20310 // ! 100m portée ! stun de 3 secondes
#define SPELL_HEAL_SHADOW_SPIRIT        24804 // A priori ok

// Timers
#define TIMER_AOE_SHADOWBOLT_LETHON     15000 // 15 secondes
#define TIMER_DESPAWN_SHADOW_SPIRIT     30000 // 30 secondes



struct shadowSpiritAI : public ScriptedAI
{   
    Unit* Lethon;
    SimpleCooldown* SCDTimerUpdateMovement;
    SimpleCooldown* SCDFreezeWhenJustSpawn;
    bool canStartMovement;
    
    shadowSpiritAI(Creature *c) : ScriptedAI(c)
    {
        SCDTimerUpdateMovement = new SimpleCooldown(1000);
        SCDFreezeWhenJustSpawn= new SimpleCooldown(1500);
    }
    void EnterCombat(Unit *who)override {}
    
    void JustDied(Unit *victim)
    override {
        // Avoid to see the corpse
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
    }
    
    void Reset()
    override {
        SCDTimerUpdateMovement->reinitCD();
        SCDFreezeWhenJustSpawn->reinitCD();
        Lethon=nullptr;
        canStartMovement=false;
                
        me->SetFloatValue(UNIT_FIELD_BOUNDINGRADIUS, 0);
        me->SetFloatValue(UNIT_FIELD_COMBATREACH, 0);
        me->SetFloatValue(UNIT_FIELD_RANGEDATTACKTIME, 0);
        
        DoCast(me,VISUAL_SPELL_SHADOW_SPIRIT,true);
    }
    
    bool UpdateVictim(bool evade)
    {
        return true;
    }
    
    void DoMeleeAttackIfReady() override
    {
        return;
    }
    
    void AttackStart(Unit* who)
    override {
        return;
    }
    
    void UpdateAI(const uint32 diff)
    override {   
        if( !Lethon )
        {
            // Il n'aime plus la vie
            me->DealDamage(me, me->GetHealth(), nullptr, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, nullptr, false);
            return;
        }
        
        if(Lethon->IsDead())
        {
            // Il n'aime plus la vie
            me->DealDamage(me, me->GetHealth(), nullptr, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, nullptr, false);
            return;
        }
        
        
        
        // Checking if the spirit can move
        if(!canStartMovement && SCDFreezeWhenJustSpawn->CheckAndUpdate(diff))
        {
            canStartMovement=true;
        }
        
        // Freeze Spirit
        if(!canStartMovement)
        {
            return;
        }
        
        // If spirit can move, he goes to Lethon and heal him
        if(SCDTimerUpdateMovement->CheckAndUpdate(diff))
                me->GetMotionMaster()->MoveFollow(Lethon,0,0);
        
        if(me->GetDistance(Lethon->GetPositionX(),Lethon->GetPositionY(),Lethon->GetPositionZ())<10)
        {
            // On donne la vie a lethon puis on se tire une balle in the head
            DoCast(Lethon,SPELL_HEAL_SHADOW_SPIRIT,true);
            me->DealDamage(me, me->GetHealth(), nullptr, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, nullptr, false);
        }
    }
    
};


struct LethonAI : public DragonOfNightmareAI_template
{
    int lowHpYellLeft; 
    SimpleCooldown* SCDAoeShadowBolt; 
    
    LethonAI(Creature *c) : DragonOfNightmareAI_template(c)
    {
        SCDAoeShadowBolt = new SimpleCooldown(TIMER_AOE_SHADOWBOLT_LETHON,1000); // On lance une SB dès le début
        
        // To make event of spirit more realist
        me->SetFloatValue(UNIT_FIELD_BOUNDINGRADIUS, 1.5f);
        me->SetFloatValue(UNIT_FIELD_COMBATREACH, 1.5f);
        me->SetFloatValue(UNIT_FIELD_RANGEDATTACKTIME, 1.5f);
    }
    
    void EnterCombat(Unit* u)
    override {
        me->Yell(YELL_ON_AGGRO_LETHON,LANG_UNIVERSAL,nullptr);
        return;
    }
    
    void Reset()
    override {
        // reset phase
        DragonOfNightmareAI_template::Reset();
        lowHpYellLeft=1;
        SCDAoeShadowBolt->resetAtStart();
    }
    
    void KilledUnit(Unit *victim)
    override {
        DragonOfNightmareAI_template::KilledUnit(victim);
        return;
    }
    
    void CastShadowSpirit()
    {
        //récupération de la liste d'aggro de Lethon
        std::list<HostileReference*>& threatList = me->getThreatManager().getThreatList();
            if (threatList.empty())
                return;
        // On prends les cibles intéressante (unit et vivantes : Lethon n'est pas nécrophile tention!)
        std::list<Unit*> targets;
        for (auto & itr : threatList) 
        {
            Unit* unit = ObjectAccessor::GetUnit((*me), itr->getUnitGuid());
            if (unit && unit->IsAlive())
                targets.push_back(unit);
        }
        // On zigouille les unités interressantes. Enfin facon de parler :p
        
        Creature* ShadowSpirit;
        for(auto & target : targets)
        {
            DoCast(target,SPELL_STUN_POP_SPIRIT,true); // On les stun 3 secondes, tant pis si pas a portée
            //(uint32 id, float x, float y, float z, float ang,TempSummonType spwtype,uint32 despwtime)
            // Summon with absolute coordinates
            ShadowSpirit=me->SummonCreature(ID_MOB_SHADOW_SPIRIT,target->GetPositionX(),target->GetPositionY(),target->GetPositionZ(),0,TEMPSUMMON_TIMED_DESPAWN,TIMER_DESPAWN_SHADOW_SPIRIT); 
            if(ShadowSpirit)
            {
                ShadowSpirit->SetFaction(me->GetFaction());
                ((shadowSpiritAI*)ShadowSpirit->AI())->Lethon=me;
            }
        }
        return;
    }
    
    
    
    void JustDied(Unit *victim)
    override {
        DragonOfNightmareAI_template::JustDied(victim);
        me->Yell(TELL_AT_DEATH_LETHON,LANG_UNIVERSAL,nullptr);
    }
    
    void UpdateAI(const uint32 diff)
    override {
        DragonOfNightmareAI_template::UpdateAI(diff);
        
        if(SCDAoeShadowBolt->CheckAndUpdate(diff))
            DoCast(me,SPELL_AOE_SHADOWBOLT_LETHON,true);
        
        if(shouldCast25PrecentSpell(NumSpell25PrecentLeft))
        {
            CastShadowSpirit();
            me->Yell(YELL_AT_PHASE_CHANGE_LETHON,LANG_UNIVERSAL,nullptr);
        }
        
        if(lowHpYellLeft && me->IsBelowHPPercent(5))
        {
            lowHpYellLeft = 0;
            me->Yell(YELL_AT_5_PRECENT_LIFE_LETHON,LANG_UNIVERSAL,nullptr);
        }
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_LethonAI(Creature *_Creature)
{
    return new LethonAI (_Creature);
}

CreatureAI* GetAI_shadowSpiritAI(Creature *_Creature)
{
    return new shadowSpiritAI (_Creature);
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////


// Emeriss //

// Define
// Spell
#define SPELL_CORRUPTION_OF_THE_EARTH_EMERISS   24910
#define SPELL_VOLATILE_INFECTION_EMERISS        24928
#define SPELL_TOXIC_CLOUD_MUSHROOM              34168

// Timer
#define TIMER_VOLATILE_INFECTION_EMERISS        10000
#define TIMER_MUSHROOM_SPAWN                    300000 // 5 min

// Creature
#define CREATURE_MUSHROOM                       17990 // Actuellement celui de basse-tourbière

// Yells
#define YELL_ON_AGGRO_EMERISS           "L'espoir n'est qu'une infection de l'âme. Ces terres se déssècheront et mourront!"
#define YELL_AT_PHASE_CHANGE_EMERISS    "Goûtez à la corruption de VOTRE terre!"
#define TELL_AT_DEATH_EMERISS           "Non... Ne laissez pas la... corruption se..."
#define YELL_AT_5_PRECENT_LIFE_EMERISS  "Mon sang... C'est impossible! Je ne me laisserais pas gagner par la corruption!"

struct EmerissAI : public DragonOfNightmareAI_template
{
    int lowHpYellLeft; 
    SimpleCooldown* SCDVolatileInfection; 
    
    EmerissAI(Creature *c) : DragonOfNightmareAI_template(c)
    {
        SCDVolatileInfection = new SimpleCooldown (TIMER_VOLATILE_INFECTION_EMERISS,1000); // On lance une SB dès le début
    }
    
    void EnterCombat(Unit* u)
    override {
        me->Yell(YELL_ON_AGGRO_EMERISS,LANG_UNIVERSAL,nullptr);
        return;
    }
    
    void Reset()
    override {
        // reset phase
        DragonOfNightmareAI_template::Reset();
        lowHpYellLeft=1;
        SCDVolatileInfection->resetAtStart();
    }
    
    void KilledUnit(Unit *victim)
    override {
        DragonOfNightmareAI_template::KilledUnit(victim);
        Creature* Mushroom=me->SummonCreature(CREATURE_MUSHROOM, victim->GetPositionX(), victim->GetPositionY(), victim->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN, TIMER_MUSHROOM_SPAWN); 
        if(Mushroom) // Bagger bagger bagger bagger Mushroom! Mushroom!
        {
                Mushroom->AI()->message(0,1); //set Stop to true
                Mushroom->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                Mushroom->SetFaction(me->GetFaction());
                Mushroom->CastSpell(Mushroom, SPELL_TOXIC_CLOUD_MUSHROOM, true);
        }
        return;
    }
    
    void JustDied(Unit *victim)
    override {
        DragonOfNightmareAI_template::JustDied(victim);
        me->Yell(TELL_AT_DEATH_EMERISS,LANG_UNIVERSAL,nullptr);
    }
    
    void UpdateAI(const uint32 diff)
    override {
        DragonOfNightmareAI_template::UpdateAI(diff);
        
        if(SCDVolatileInfection->CheckAndUpdate(diff))
            DoCast(SelectTarget(SELECT_TARGET_RANDOM,0),SPELL_VOLATILE_INFECTION_EMERISS,true);
        
        if(shouldCast25PrecentSpell(NumSpell25PrecentLeft))
        {
            me->Yell(YELL_AT_PHASE_CHANGE_EMERISS,LANG_UNIVERSAL,nullptr);            
            DoCast(me,SPELL_CORRUPTION_OF_THE_EARTH_EMERISS,true);
        }
        
        if(lowHpYellLeft && me->IsBelowHPPercent(5))
        {
            lowHpYellLeft = 0;
            me->Yell(YELL_AT_5_PRECENT_LIFE_EMERISS,LANG_UNIVERSAL,nullptr);
        }
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_emeriss(Creature *_Creature)
{
    return new EmerissAI (_Creature);
}


////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////


// taerar //

// Spell
#define SPELL_POISONCLOUD_SHADES        24840
#define SPELL_POISONBREATH_SHADES       20667
#define SPELL_ARCANEBLAST_TAERAR        24857
#define SPELL_BELLOWINGROAR_TAERAR      22686
#define SPELL_BANISH_TAERAR             44836

// Timer
#define TIMER_POISONCLOUD_SHADES        20000
#define TIMER_POISONBREATH_SHADES       10000
#define TIMER_ARCANEBLAST_TAERAR        8000
#define TIMER_BELLOWINGROAR_TAERAR      25000
#define TIMER_DESPAWN_SHADES_OF_TAERAR  900000 // 15 minutes
#define TIMER_BANISH_TAERAR             60000

// Yells
#define YELL_ON_AGGRO_TAERAR           "La paix n'est un rêve éphémère! Que le CAUCHEMARD commence!"
#define YELL_AT_PHASE_CHANGE_TAERAR    "Vous n'êtes que les rejeton de la folie pure. Laissez moi vous libérer!"
#define TELL_AT_DEATH_TAERAR           "Ce n'est... Ce n'est pas encore... fini... Ma magie... vous exterminera... tous..."
#define YELL_AT_5_PRECENT_LIFE_TAERAR  "Jamais je ne me laisserais abbatre! Vous entendez? JAMAIS!"

// Creature
#define ID_MOB_SHADES_OF_TAERAR         15302

// Sizes
#define size_TabShadesOfTaerar          3

struct boss_shadeoftaerarAI : public ScriptedAI
{
    
    SimpleCooldown* SCDPoisonCLoud;
    SimpleCooldown* SCDPoisonBreath;
    
    Creature* Taerar;
    
    boss_shadeoftaerarAI(Creature *c) : ScriptedAI(c) 
    {
        SCDPoisonCLoud = new SimpleCooldown(TIMER_POISONCLOUD_SHADES);
        SCDPoisonBreath = new SimpleCooldown(TIMER_POISONBREATH_SHADES);
    }

    void Instakill(Unit* Target)
    {
        Target->DealDamage(Target, Target->GetHealth(), nullptr, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, nullptr, false);
    }

    void Reset()
    override {
        SCDPoisonCLoud->resetAtStart();
        SCDPoisonBreath->resetAtStart();
    }

    void EnterCombat(Unit *who)
    override {
    }

    void UpdateAI(const uint32 diff)
    override {
        if(!Taerar || Taerar->IsDead() || !Taerar->IsInCombat() )
        {
            // Suicide
            Instakill(me);
            me->RemoveCorpse();
        }
        
        if (!UpdateVictim())
            return;
        
        if(SCDPoisonCLoud->CheckAndUpdate(diff))
            DoCast(me->GetVictim(),SPELL_POISONCLOUD_SHADES);
        
        if(SCDPoisonBreath->CheckAndUpdate(diff))
            DoCast(me->GetVictim(),SPELL_POISONBREATH_SHADES);

        DoMeleeAttackIfReady();
    }
    
    void KilledUnit(Unit *victim)
    override {
        if(!victim)
            return;
        // If someone is killed he takes the aura
        ((DragonOfNightmareAI_template*)Taerar->AI())->KilledUnit(victim);
    }
};


struct boss_taerarAI : public DragonOfNightmareAI_template
{
    int lowHpYellLeft;
    SimpleCooldown* SCDArcaneBlast;
    SimpleCooldown* SCDBellowingGroar;
    SimpleCooldown* SCDBanish;
    bool isBanished;
    
    Creature *TabShadesOfTaerar[size_TabShadesOfTaerar];
    
    void resetShadesPointer()
    {
        for(auto & i : TabShadesOfTaerar)
        {       // Evite un crash lors du pop du boss. 
            i=nullptr;
        }
    }
    
    boss_taerarAI(Creature *c) : DragonOfNightmareAI_template(c)
    {        
        SCDArcaneBlast = new SimpleCooldown(TIMER_ARCANEBLAST_TAERAR);
        SCDBellowingGroar = new SimpleCooldown(TIMER_BELLOWINGROAR_TAERAR);
        SCDBanish = new SimpleCooldown(TIMER_BANISH_TAERAR);
        resetShadesPointer();
    }
    
    void EnterCombat(Unit* u)
    override {
        me->Yell(YELL_ON_AGGRO_TAERAR,LANG_UNIVERSAL,nullptr);
        return;
    }
    
    void Reset()
    override {
        resetShadesPointer();
        // reset phase
        lowHpYellLeft = 1;
        isBanished=false;
        
        SCDArcaneBlast->resetAtStart();
        SCDBellowingGroar->resetAtStart();
        SCDBanish->resetAtStart();
        
        me->RemoveAurasDueToSpellByCancel(SPELL_BANISH_TAERAR);
        DragonOfNightmareAI_template::Reset();
    }
    
    void KilledUnit(Unit *victim)
    override {
        DragonOfNightmareAI_template::KilledUnit(victim);
    }
    
    void JustDied(Unit *victim)
    override {
        DragonOfNightmareAI_template::JustDied(victim);
        me->Yell(TELL_AT_DEATH_TAERAR,LANG_UNIVERSAL,nullptr);
    }
    
    void UpdateAI(const uint32 diff)
    override {
        if(!UpdateVictim())
            return;
        
        if(isBanished)
        {
            if(SCDBanish->CheckAndUpdate(diff) || checkIfAllShadesAreDead())
            {
                me->RemoveAurasDueToSpellByCancel(SPELL_BANISH_TAERAR);
                SCDBanish->resetAtStart();
                isBanished = false;
            }
            return; // freeze
        }
        
        DragonOfNightmareAI_template::UpdateAI(diff);
        
        if(SCDArcaneBlast->CheckAndUpdate(diff))
            DoCast(SelectTarget(SELECT_TARGET_RANDOM,0),SPELL_ARCANEBLAST_TAERAR,true);
        
        if(SCDBellowingGroar->CheckAndUpdate(diff))
            DoCast(me,SPELL_BELLOWINGROAR_TAERAR,false); // if true, no animation :'(
        
        if(shouldCast25PrecentSpell(NumSpell25PrecentLeft))
        {
            me->Yell(YELL_AT_PHASE_CHANGE_TAERAR,LANG_UNIVERSAL,nullptr);            
            spawnShadesOfTaerar();
            isBanished=true;
        }
        
        if(lowHpYellLeft && me->IsBelowHPPercent(5))
        {
            lowHpYellLeft = 0;
            me->Yell(YELL_AT_5_PRECENT_LIFE_TAERAR,LANG_UNIVERSAL,nullptr);
        }
        DoMeleeAttackIfReady();
    }
    
    void spawnShadesOfTaerar()
    {
        if(!me->IsInCombat()) // évite l'invocation de shades au reset (arrive souvent)
        {
            return;
        }
        // Cas de figure peu probable : arriver a réinvoquer des shades 
        // alors que les précédent ne sont pas mort
        //killAllShades(); 
        
        Creature* pShadesOfTaerar;
        boss_shadeoftaerarAI *ShadesOfTaerarAICasted;
        for(auto & i : TabShadesOfTaerar)
        {
            pShadesOfTaerar=DoSpawnCreature(ID_MOB_SHADES_OF_TAERAR,rand()%40,rand()%40,0,0,TEMPSUMMON_TIMED_DESPAWN,TIMER_DESPAWN_SHADES_OF_TAERAR);
            if(pShadesOfTaerar)
            {
                i = pShadesOfTaerar;
                ShadesOfTaerarAICasted = (boss_shadeoftaerarAI*)pShadesOfTaerar->AI();
                ShadesOfTaerarAICasted->Taerar=me;
                
                i->SetFaction(me->GetFaction());
                Unit* target=SelectTarget(SELECT_TARGET_RANDOM,0);
                i->AI()->AttackStart(target); // Si true ils ne se déplacent po :(
                i->AddThreat(target,10000.0f);
                
            }
        }
        me->InterruptNonMeleeSpells(false);
        me->AddAura(SPELL_BANISH_TAERAR,me);
        return;
    }
    
    bool checkIfAllShadesAreDead()
    {
        bool AllShadesDead = true;
        for(auto & i : TabShadesOfTaerar) // Les mort sont trop NULL :D
        {
            if(i)
            {
                if( i->IsDead())
                {
                    i=nullptr;
                }
                else
                {
                    AllShadesDead=false; // he is alive
                }
            }
            
        }
        return AllShadesDead;
    }
};


CreatureAI* GetAI_boss_taerar(Creature *_Creature)
{
    return new boss_taerarAI (_Creature);
}

CreatureAI* GetAI_boss_shadeoftaerar(Creature *_Creature)
{
    return new boss_shadeoftaerarAI (_Creature);
}


////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

// Ysondre //

// Spells
#define SPELL_LIGHTNINGWAVE_YSONDRE     24819
#define SPELL_SUMMONDRUIDS_YSONDRE      24795
#define SPELL_MOONFIRE_DRUID            21669
#define SPELL_CURSE_OF_THORNS_DRUID     16247
#define SPELL_SILENCE_DRUID             6726

// Timers
#define TIMER_LITHGNING_WAVE_YSONDRE    7500 // 7.5 secondes
#define TIMER_MOONFIRE_DRUID            4000 // 4 secondes
#define TIMER_CURSE_OF_THORNS_DRUID     10000 // 10s
#define TIMER_SILENCE_DRUID             6000 // 6s
#define TIMER_SPAWN_DRUID               600000 // 10 minutes

// Yells
#define YELL_ON_AGGRO_YSONDRE           "L'esence même de la vie est touchée. Que les rêveurs soient vengés!"
#define YELL_AT_PHASE_CHANGE_YSONDRE    "Venez à moi, ô rêveurs - et réclamez votre vengeance!"
#define TELL_AT_DEATH_YSONDRE           "Non... Impossible... Les rêveurs...  Ils doivent... "
#define YELL_AT_5_PRECENT_LIFE_YSONDRE  "C'est impossible...je suis plus puissant que vous... rien ne peut m'arrêter!"

// Creatures
#define NUMBER_OF_DRUIDS                10
#define ID_MOD_DRUID_YSONDRE            15260



struct npc_dementeddruidsAI : public ScriptedAI
{
    SimpleCooldown *SCDMoonFire;
    SimpleCooldown *SCDCurseOfThorns;
    SimpleCooldown *SCDSilence;
    Creature* Ysondre;
    
    void Instakill(Unit* Target)
    {
        Target->DealDamage(Target, Target->GetHealth(), nullptr, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, nullptr, false);
    }
    
    npc_dementeddruidsAI(Creature *c) : ScriptedAI(c) 
    {
        SCDMoonFire=new SimpleCooldown(TIMER_MOONFIRE_DRUID);
        SCDCurseOfThorns=new SimpleCooldown(TIMER_CURSE_OF_THORNS_DRUID);
        SCDSilence=new SimpleCooldown(TIMER_SILENCE_DRUID);
    }

    void Reset()
    override {
        SCDMoonFire->resetAtStart();
        SCDCurseOfThorns->resetAtStart();
        SCDSilence->resetAtStart();
    }

    void EnterCombat(Unit *who)
    override {
    }

    void KilledUnit(Unit *victim)
    override {
        if(!victim)
            return;
        // If someone is killed he takes the aura
        ((DragonOfNightmareAI_template*)Ysondre->AI())->KilledUnit(victim);
    }
    
    void UpdateAI(const uint32 diff)
    override {
        if (!UpdateVictim())
            return;

        if(!Ysondre)
        {
            Ysondre=ObjectAccessor::GetCreature(*me,me->GetOwnerGUID());
            return;
        }
        
        if(Ysondre->IsDead() || !Ysondre->IsInCombat())
        {
            Instakill(me);
        }
        
        if(SCDMoonFire->CheckAndUpdate(diff))
            DoCast(me->GetVictim(),SPELL_MOONFIRE_DRUID,false);
        
        
        Unit* TargetRandom;
        if(SCDCurseOfThorns->CheckAndUpdate(diff))
        {
            if((TargetRandom = SelectTarget(SELECT_TARGET_RANDOM,0)))
                DoCast(TargetRandom,SPELL_CURSE_OF_THORNS_DRUID,false);
        }
            
        
        if(SCDSilence->CheckAndUpdate(diff))
        {
            if((TargetRandom = SelectTarget(SELECT_TARGET_RANDOM,0)))
                DoCast(TargetRandom,SPELL_SILENCE_DRUID,false);
        }

        DoMeleeAttackIfReady();
    }
};



struct boss_ysondreAI : public DragonOfNightmareAI_template
{
    
    SimpleCooldown *SCDLightningWave;
    
    boss_ysondreAI(Creature *c) : DragonOfNightmareAI_template(c) 
    {
        SCDLightningWave=new SimpleCooldown(TIMER_LITHGNING_WAVE_YSONDRE);
    }
    
    void JustDied(Unit *victim)
    override {
        me->Yell(TELL_AT_DEATH_YSONDRE,LANG_UNIVERSAL,nullptr);
        DragonOfNightmareAI_template::JustDied(victim);
    }
    
    void KilledUnit(Unit *victim)
    override {
        DragonOfNightmareAI_template::KilledUnit(victim);
    }

    void Reset()
    override {
        DragonOfNightmareAI_template::Reset();
        SCDLightningWave->resetAtStart();
    }

    void EnterCombat(Unit *who)
    override {
        me->Yell(YELL_ON_AGGRO_YSONDRE,LANG_UNIVERSAL,nullptr);
    }

    void SummonDruids()
    {
        Unit* Target;
        npc_dementeddruidsAI* Druid;
        Creature* DruidCreature;
        for(int i=0 ; i<10 ; i++)
        {
            Target = SelectTarget(SELECT_TARGET_RANDOM,0);
            if(Target)
            {
                // Summon with absolute coordinate
                DruidCreature = me->SummonCreature(ID_MOD_DRUID_YSONDRE,Target->GetPositionX()+rand()%10,Target->GetPositionY()+rand()%10,Target->GetPositionZ(),0,TEMPSUMMON_TIMED_DESPAWN,TIMER_SPAWN_DRUID);
                if(DruidCreature)
                {
                    Druid=((npc_dementeddruidsAI*)DruidCreature->AI());
                    Druid->Ysondre=me;
                    Druid->AttackStart(Target);
                    DruidCreature->AddThreat(Target,6000.0f);
                }
            }
        }
    }

    void UpdateAI(const uint32 diff)
    override {
        DragonOfNightmareAI_template::UpdateAI(diff);

        if(SCDLightningWave->CheckAndUpdate(diff))
                DoCast(SelectTarget(SELECT_TARGET_RANDOM,0),SPELL_LIGHTNINGWAVE_YSONDRE,true);

        if(shouldCast25PrecentSpell(NumSpell25PrecentLeft))
        {
                me->Yell(YELL_AT_PHASE_CHANGE_YSONDRE,LANG_UNIVERSAL,nullptr);
                SummonDruids();
        }
                
        if(lowHpYellLeft && me->IsBelowHPPercent(5))
        {
            lowHpYellLeft = 0;
            me->Yell(YELL_AT_5_PRECENT_LIFE_YSONDRE,LANG_UNIVERSAL,nullptr);
        }
                
        DoMeleeAttackIfReady();
    }
};



CreatureAI* GetAI_boss_ysondre(Creature *_Creature)
{
    return new boss_ysondreAI (_Creature);
}

CreatureAI* GetAI_npc_dementeddruidsAI(Creature *_Creature)
{
    return new npc_dementeddruidsAI (_Creature);
}




////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////


// Addsc() //


void AddSC_boss_dragonsofnightmare()
{
    OLDScript *newscript;
    
    newscript = new OLDScript;
    newscript->Name="npc_dreamfog";
    newscript->GetAI = &GetAI_DreamFog;
    sScriptMgr->RegisterOLDScript(newscript);
    
    newscript = new OLDScript;
    newscript->Name="boss_lethon";
    newscript->GetAI = &GetAI_LethonAI;
    sScriptMgr->RegisterOLDScript(newscript);
    
    newscript = new OLDScript;
    newscript->Name="npc_spiritshade";
    newscript->GetAI = &GetAI_shadowSpiritAI;
    sScriptMgr->RegisterOLDScript(newscript);
    
    newscript = new OLDScript;
    newscript->Name="boss_emeriss";
    newscript->GetAI = &GetAI_boss_emeriss;
    sScriptMgr->RegisterOLDScript(newscript);
    
    newscript = new OLDScript;
    newscript->Name="boss_taerar";
    newscript->GetAI = &GetAI_boss_taerar;
    sScriptMgr->RegisterOLDScript(newscript);

    newscript = new OLDScript;
    newscript->Name="boss_shade_of_taerar";
    newscript->GetAI = &GetAI_boss_shadeoftaerar;
    sScriptMgr->RegisterOLDScript(newscript);
    
    newscript = new OLDScript;
    newscript->Name="boss_ysondre";
    newscript->GetAI = &GetAI_boss_ysondre;
    sScriptMgr->RegisterOLDScript(newscript);

    newscript = new OLDScript;
    newscript->Name="mob_dementeddruids";
    newscript->GetAI = &GetAI_npc_dementeddruidsAI;
    sScriptMgr->RegisterOLDScript(newscript);
}
