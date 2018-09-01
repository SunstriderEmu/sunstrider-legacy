#include "PlayerTaxi.h"
#include "GameEventMgr.h"

//== PlayerTaxi ================================================
PlayerTaxi::PlayerTaxi() : m_flightMasterFactionId(0) 
{ 
    m_taximask.fill(0); 
}

void PlayerTaxi::InitTaxiNodesForLevel(uint32 race, uint32 level)
{
    // capital and taxi hub masks
    switch(race)
    {
        case RACE_HUMAN:         SetTaximaskNode(TAXI_STORMWIND);    break; 
        case RACE_ORC:           SetTaximaskNode(TAXI_ORGRIMMAR);    break; 
        case RACE_DWARF:         SetTaximaskNode(TAXI_IRONFORGE);    break; 
        case RACE_NIGHTELF:      SetTaximaskNode(TAXI_AUBERDINE);    
                                 SetTaximaskNode(TAXI_RUTHERAN);     break; 
        case RACE_UNDEAD_PLAYER: SetTaximaskNode(TAXI_UNDERCITY);    break; 
        case RACE_TAUREN:        SetTaximaskNode(TAXI_THUNDERBLUFF); break; 
        case RACE_GNOME:         SetTaximaskNode(TAXI_IRONFORGE);    break; 
        case RACE_TROLL:         SetTaximaskNode(TAXI_ORGRIMMAR);    break; 
        case RACE_BLOODELF:      SetTaximaskNode(TAXI_SILVERMOON);   break; 
        case RACE_DRAENEI:       SetTaximaskNode(TAXI_EXODAR);       break; 
    }
    // new continent starting masks (It will be accessible only at new map)
    switch(Player::TeamForRace(race))
    {
        case ALLIANCE: SetTaximaskNode(100); break;
        case HORDE:    SetTaximaskNode(99);  break;
    }
    // level dependent taxi hubs
    if(level>=68)
        SetTaximaskNode(213);                               //Shattered Sun Staging Area
}

void PlayerTaxi::LoadTaxiMask(const char* data)
{
    Tokens tokens = StrSplit(data, " ");

    int index;
    Tokens::iterator iter;
    for (iter = tokens.begin(), index = 0;
        (index < TaxiMaskSize) && (iter != tokens.end()); ++iter, ++index)
    {
        // load and set bits only for existed taxi nodes
        m_taximask[index] = sTaxiNodesMask[index] & uint32(atol((*iter).c_str()));
    }
}

void PlayerTaxi::AppendTaximaskTo( ByteBuffer& data, bool all )
{
    if(all)
    {
        for (uint32 i : sTaxiNodesMask)
            data << uint32(i);              // all existed nodes
    }
    else
    {
        //hackz to disable Shattered Sun Staging Area until patch 2.4 is enabled
        bool patch24active = sGameEventMgr->IsActiveEvent(GAME_EVENT_2_4);
        for (uint8 i = 0; i < TaxiMaskSize; i++)
        {
            if (!patch24active && i == 6)
                data << uint32(m_taximask[i] & ~0x100000);      //Shattered Sun Staging Area
            else
                data << uint32(m_taximask[i]);                  // known nodes
        }
    }
}

bool PlayerTaxi::IsTaximaskNodeKnown(uint32 nodeidx) const
{
    //hackz to disable Shattered Sun Staging Area until patch 2.4 is enabled
    if (nodeidx == 213)
    {
        bool patch24active = sGameEventMgr->IsActiveEvent(GAME_EVENT_2_4);
        if (!patch24active)
            return false;
    }

    uint8  field = uint8((nodeidx - 1) / 32);
    uint32 submask = 1 << ((nodeidx - 1) % 32);
    return (m_taximask[field] & submask) == submask;
}

bool PlayerTaxi::LoadTaxiDestinationsFromString( const std::string& values, uint32 team)
{
    ClearTaxiDestinations();

    Tokenizer tokens(values, ' ');
    auto iter = tokens.begin();
    if (iter != tokens.end())
        m_flightMasterFactionId = atoul(*iter);

    ++iter;
    for (; iter != tokens.end(); ++iter)
    {
        uint32 node = atoul(*iter);
        AddTaxiDestination(node);
    }

    if(m_TaxiDestinations.empty())
        return true;

    // Check integrity
    if(m_TaxiDestinations.size() < 2)
        return false;

    for(size_t i = 1; i < m_TaxiDestinations.size(); ++i)
    {
        uint32 cost;
        uint32 path;
        sObjectMgr->GetTaxiPath(m_TaxiDestinations[i-1],m_TaxiDestinations[i],path,cost);
        if(!path)
            return false;
    }

    // can't load taxi path without mount set (quest taxi path?)
    if (!sObjectMgr->GetTaxiMountDisplayId(GetTaxiSource(), team, true))
        return false;

    return true;
}

std::string PlayerTaxi::SaveTaxiDestinationsToString()
{
    if(m_TaxiDestinations.empty())
        return "";

    std::ostringstream ss;
    ss << m_flightMasterFactionId << ' ';

    for(uint32 m_TaxiDestination : m_TaxiDestinations)
        ss << m_TaxiDestination << " ";

    return ss.str();
}

uint32 PlayerTaxi::GetCurrentTaxiPath() const
{
    if(m_TaxiDestinations.size() < 2)
        return 0;

    uint32 path;
    uint32 cost;

    sObjectMgr->GetTaxiPath(m_TaxiDestinations[0],m_TaxiDestinations[1],path,cost);

    return path;
}

FactionTemplateEntry const* PlayerTaxi::GetFlightMasterFactionTemplate() const
{
    return sFactionTemplateStore.LookupEntry(m_flightMasterFactionId);
}

std::ostringstream& operator<<(std::ostringstream& ss, PlayerTaxi const& taxi)
{
    for (uint8 i = 0; i < TaxiMaskSize; ++i)
        ss << taxi.m_taximask[i] << ' ';
    return ss;
}
