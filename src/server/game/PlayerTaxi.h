#ifndef __PLAYERTAXI_H__
#define __PLAYERTAXI_H__

enum TaxiNodes
{
    // Alliance
    TAXI_STORMWIND                  = 2,
    TAXI_IRONFORGE                  = 6,
    TAXI_SOUTHSHORE                 = 14,
    TAXI_BOOTY_BAY_ALLIANCE         = 19,
    TAXI_AUBERDINE                  = 26,
    TAXI_RUTHERAN                   = 27,
    TAXI_ASTRANAAR                  = 28,
    TAXI_THERAMODE                  = 32,
    TAXI_GADGETZAN_ALLIANCE         = 39,
    TAXI_FEATHERMOON                = 41,
    TAXI_HINTERLANDS                = 43,
    TAXI_NETHERGARDE_KEEP           = 45,
    TAXI_EVERLOOK_ALLIANCE          = 52,
    TAXI_EXODAR                     = 94,
    TAXI_STRANGLETHORN_REBEL_CAMP   = 195,
    
    // Horde
    TAXI_UNDERCITY                  = 11,
    TAXI_HAMMERFALL                 = 17,
    TAXI_BOOTY_BAY_HORDE            = 18,
    TAXI_KARGATH                    = 21,
    TAXI_GROMGOL                    = 20,
    TAXI_THUNDERBLUFF               = 22,
    TAXI_ORGRIMMAR                  = 23,
    TAXI_THOUSAND_NEEDLES           = 30,
    TAXI_GADGETZAN_HORDE            = 40,
    TAXI_EVERLOOK_HORDE             = 53,
    TAXI_STONARD                    = 56,
    TAXI_SPLINTERTREE               = 61,
    TAXI_TAURAJO                    = 77,
    TAXI_SILVERMOON                 = 82,
    TAXI_TRAQUILIEN                 = 83,
};

class TC_GAME_API PlayerTaxi
{
    public:
        PlayerTaxi();
        ~PlayerTaxi() = default;
        // Nodes
        void InitTaxiNodesForLevel(uint32 race, uint32 level);
        void LoadTaxiMask(const char* data);

        uint32 GetTaximask( uint8 index ) const { return m_taximask[index]; }
        bool IsTaximaskNodeKnown(uint32 nodeidx) const;
        void ResetTaximask() {
            for (auto& i : m_taximask)
                i = 0;
        }
        bool SetTaximaskNode(uint32 nodeidx)
        {
            uint8  field   = uint8((nodeidx - 1) / 32);
            uint32 submask = 1<<((nodeidx-1)%32);
            if ((m_taximask[field] & submask) != submask )
            {
                m_taximask[field] |= submask;
                return true;
            }
            else
                return false;
        }
        void AppendTaximaskTo(ByteBuffer& data,bool all);

        // Destinations
        bool LoadTaxiDestinationsFromString(const std::string& values, uint32 team);
        std::string SaveTaxiDestinationsToString();

        void ClearTaxiDestinations() { m_TaxiDestinations.clear(); }
        void AddTaxiDestination(uint32 dest) { m_TaxiDestinations.push_back(dest); }
        uint32 GetTaxiSource() const { return m_TaxiDestinations.empty() ? 0 : m_TaxiDestinations.front(); }
        uint32 GetTaxiDestination() const { return m_TaxiDestinations.size() < 2 ? 0 : m_TaxiDestinations[1]; }
        uint32 GetCurrentTaxiPath() const;
        uint32 NextTaxiDestination()
        {
            m_TaxiDestinations.pop_front();
            return GetTaxiDestination();
        }
        std::deque<uint32> const& GetPath() const { return m_TaxiDestinations; }
        bool empty() const { return m_TaxiDestinations.empty(); }
        FactionTemplateEntry const* GetFlightMasterFactionTemplate() const;
        void SetFlightMasterFactionTemplateId(uint32 factionTemplateId) { m_flightMasterFactionId = factionTemplateId; }

        friend std::ostringstream& operator<<(std::ostringstream& ss, PlayerTaxi const& taxi);
    private:
        TaxiMask m_taximask;
        std::deque<uint32> m_TaxiDestinations;
        uint32 m_flightMasterFactionId;
};

std::ostringstream& operator<<(std::ostringstream& ss, PlayerTaxi const& taxi);

#endif
