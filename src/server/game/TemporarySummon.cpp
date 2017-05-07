
#include "TemporarySummon.h"
#include "WorldPacket.h"
#include "MapManager.h"
#include "Log.h"
#include "ObjectAccessor.h"
#include "CreatureAI.h"

TemporarySummon::TemporarySummon( uint64 summoner ) :
Creature(), m_type(TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN), m_timer(0), m_lifetime(0), m_summoner(summoner)
{
    m_unitTypeMask |= UNIT_MASK_SUMMON;
}

void TemporarySummon::Update( uint32 diff )
{
    if (m_deathState == DEAD)
    {
        UnSummon();
        return;
    }
    switch(m_type)
    {
        case TEMPSUMMON_MANUAL_DESPAWN:
            break;
        case TEMPSUMMON_TIMED_DESPAWN:
        {
            if (m_timer <= diff)
            {
                UnSummon();
                return;
            }

            m_timer -= diff;
            break;
        }
        case TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT:
        {
            if (!IsInCombat())
            {
                if (m_timer <= diff)
                {
                    UnSummon();
                    return;
                }

                m_timer -= diff;
            }
            else if (m_timer != m_lifetime)
                m_timer = m_lifetime;

            break;
        }

        case TEMPSUMMON_CORPSE_TIMED_DESPAWN:
        {
            if ( m_deathState == CORPSE)
            {
                if (m_timer <= diff)
                {
                    UnSummon();
                    return;
                }

                m_timer -= diff;
            }
            break;
        }
        case TEMPSUMMON_CORPSE_DESPAWN:
        {
            // if m_deathState is DEAD, CORPSE was skipped
            if ( m_deathState == CORPSE || m_deathState == DEAD)
            {
                UnSummon();
                return;
            }

            break;
        }
        case TEMPSUMMON_DEAD_DESPAWN:
        {
            if ( m_deathState == DEAD )
            {
                UnSummon();
                return;
            }
            break;
        }
        case TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN:
        {
            // if m_deathState is DEAD, CORPSE was skipped
            if ( m_deathState == CORPSE || m_deathState == DEAD)
            {
                UnSummon();
                return;
            }

            if (m_timer <= diff)
            {
                UnSummon();
                return;
            } else m_timer -= diff;

            break;
        }
        case TEMPSUMMON_TIMED_OR_DEAD_DESPAWN:
        {
            // if m_deathState is DEAD, CORPSE was skipped
            if (m_deathState == DEAD)
            {
                UnSummon();
                return;
            }

            if (!IsInCombat() && IsAlive() )
            {
                if (m_timer <= diff)
                {
                    UnSummon();
                    return;
                }
                else
                    m_timer -= diff;
            }
            else if (m_timer != m_lifetime)
                m_timer = m_lifetime;
            break;
        }
        default:
            UnSummon();
            TC_LOG_ERROR("FIXME","Temporary summoned creature (entry: %u) have unknown type %u of ",GetEntry(),m_type);
            break;
    }

    Creature::Update( diff );
}

void TemporarySummon::Summon(TempSummonType type, uint32 lifetime, Map* map)
{
    m_type = type;
    m_timer = lifetime;
    m_lifetime = lifetime;

    AIM_Initialize();

	map->AddToMap(this->ToCreature(), true);

    Unit* owner = GetSummoner();
    if (owner)
    {
        if (IsTrigger() && m_spells[0])
        {
            SetFaction(owner->GetFaction());
            SetLevel(owner->GetLevel());
            if (owner->GetTypeId() == TYPEID_PLAYER)
                m_ControlledByPlayer = true;
        }

        if (owner->GetTypeId() == TYPEID_PLAYER)
            m_CreatedByPlayer = true;
    }
}

void TemporarySummon::UnSummon()
{
    AddObjectToRemoveList();

    Unit* sum = m_summoner ? ObjectAccessor::GetUnit(*this, m_summoner) : nullptr;
    if (sum  && sum->GetTypeId() == TYPEID_UNIT && (sum->ToCreature())->IsAIEnabled)
        (sum->ToCreature())->AI()->SummonedCreatureDespawn(this);
}

bool TemporarySummon::DespawnOnDeath()
{
    if(  m_type == TEMPSUMMON_CORPSE_DESPAWN 
      || m_type == TEMPSUMMON_DEAD_DESPAWN 
      || m_type == TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN 
      || m_type == TEMPSUMMON_TIMED_OR_DEAD_DESPAWN )
      return true;

    return false;
}